/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hTypes.h"
#include "base/hRendererConstants.h"
#include "base/hMemory.h"
#include "base/hStringUtil.h"
#include "core/hSystem.h"
#include "base/hThread.h"
#include "base/hThreadLocalStorage.h"
#include "base/hMutexAutoScope.h"
#include "base/hSemaphore.h"
#include "render/hRenderer.h"
#include "render/hIndexBufferFlags.h"
#include "render/hVertexBufferFlags.h"
#include "render/hRenderCallDesc.h"
#include "render/hMipDesc.h"
#include "render/hTextureFormat.h"
#include "render/hUniformBufferFlags.h"
#include "render/hRenderCallDesc.h"
#include "render/hProgramReflectionInfo.h"
#include "render/hRenderPrim.h"
#include "cryptoMurmurHash.h"
#include "lfds/lfds.h"
#include <d3d9.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <memory>
#include <unordered_map>
#include <functional>

void* operator new(size_t size){
    return Heart::hMalloc(size);
}

void* operator new[](size_t size) {
    return Heart::hMalloc(size);
}

void operator delete(void* ptr) {
    return Heart::hFree(ptr);
}

void operator delete[](void* ptr) {
    return Heart::hFree(ptr);
}

namespace Heart {    
namespace hRenderer {
namespace d3d9 {
    hUint getD3DFormatPitch(hUint32, D3DFORMAT);
}

    static const hUint  FRAME_COUNT = 3;
    static const hUint  RMEM_COUNT = FRAME_COUNT+1;
    static const hUint32 BC_PITCH = 0x80000000;

    hIConfigurationVariables* configVars;

    LPDIRECT3D9 pD3D;
    LPDIRECT3DDEVICE9 d3dDevice;
    bool multiThreadedRenderer;
    hUint32 fenceCount;
    hUint32 renderCommandThreshold;

    lfds_queue_state*   cmdListQueue;
    lfds_queue_state*   destructionQueue;

    hMutex              memAccess;
    hUint               memIndex;
    hUint               renderScratchMemSize;
    hByte*              renderScratchMem[RMEM_COUNT];
    hSize_t             renderScratchAllocd[RMEM_COUNT];

    hRenderFence*        fences;
    hUint                fenceIndex;
    hRenderFence*        frameFences[FRAME_COUNT];
    lfds_freelist_state* fenceFreeList;

    hThread             renderThread;
    hConditionVariable  rtComsSignal;
    hMutex              rtMutex;
    void*               rtID;

    std::unordered_map<hUint32, hUint32> d3dFormatPitch;

    // !!JM TODO: Improve these (make lock-free?), they are placeholder
    void* rtmp_malloc(hSize_t size, hUint alignment=16) {
        hMutexAutoScope sentry(&memAccess);
        renderScratchAllocd[memIndex] = hAlign(renderScratchAllocd[memIndex], alignment);
        void* r = renderScratchMem[memIndex] + renderScratchAllocd[memIndex];
        renderScratchAllocd[memIndex] += size;
        hcAssertMsg(renderScratchAllocd[memIndex] < renderScratchMemSize, "Renderer ran out of scratch memory. This is fatal");
        return r;
    }

    void rtmp_frameend() {
        hMutexAutoScope sentry(&memAccess);
        renderScratchAllocd[memIndex] = 0;
        memIndex = (memIndex+1)%RMEM_COUNT;
    }

    template<typename t_ty>
    struct IUnknownDeleter { // deleter
        void operator()(t_ty* p) const {
            p->Release();
        };
    };

    typedef std::unique_ptr<IDirect3DVertexShader9, IUnknownDeleter<IDirect3DVertexShader9>> IDirect3DVertexShader9Ptr;
    typedef std::unique_ptr<IDirect3DPixelShader9, IUnknownDeleter<IDirect3DPixelShader9>> IDirect3DPixelShader9Ptr;
    typedef std::unique_ptr<IDirect3DTexture9, IUnknownDeleter<IDirect3DTexture9>> IDirect3DTexture9Ptr;
    typedef std::unique_ptr<IDirect3DVertexBuffer9, IUnknownDeleter<IDirect3DVertexBuffer9>> IDirect3DVertexBuffer9Ptr;
    typedef std::unique_ptr<IDirect3DIndexBuffer9, IUnknownDeleter<IDirect3DIndexBuffer9>> IDirect3DIndexBuffer9Ptr;
    typedef std::unique_ptr<IDirect3DVertexDeclaration9, IUnknownDeleter<IDirect3DVertexDeclaration9>> IDirect3DVertexDeclaration9Ptr;

    struct hShaderStage {
        enum ConstantType {
            Bool, Int4, Float4, Sampler
        };
        struct ReflectionHeader {
            hUint32 magic; //0x7777C0DE
            hUint32 totalLen;
            hUint32 constCount;
        };
        struct Constant {
            hUint32 type; //0 = bool, 1 = int, 2 = float, 3 = sampler
            hUint32 regStart;
            hUint32 regCount;
            hUint32 rows;
            hUint32 columns;
            hUint32 elements;
            hUint32 members;
            hUint32 bytes;
            hUint32 stringTableOffset;
        };

        hShaderStage(hShaderProfile in_profile, const hChar* shader_prog, hUint32 len) 
            : profile(in_profile)
            , floatRegStart(~0)
            , floatRegCount(0)
            , intRegStart(~0)
            , intRegCount(0) {
            shaderBlob.reset(new hChar[len]);
            hMemCpy(shaderBlob.get(), shader_prog, len);
            header = (ReflectionHeader*)shaderBlob.get();
            constants = (Constant*)(shaderBlob.get()+sizeof(ReflectionHeader));
            strings = (char*)(constants+header->constCount);
            programBlob = (DWORD*)(shaderBlob.get()+header->totalLen);
            hUint32 float_reg_size_bytes = 0;
            hUint32 int_reg_size_bytes = 0;
            for (hUint32 i=0; i<header->constCount; ++i) {
                if (constants[i].type == 2) {
                    if (constants[i].regStart < floatRegStart) floatRegStart = constants[i].regStart;
                    auto data_end = constants[i].regStart*sizeof(float)*4+constants[i].bytes;
                    if (data_end > float_reg_size_bytes) float_reg_size_bytes = (hUint32)data_end;
                } else if (constants[i].type == 1) {
                    if (constants[i].regStart < intRegStart) intRegStart = constants[i].regStart;
                    auto data_end = constants[i].regStart*sizeof(hInt)*4+constants[i].bytes;
                    if (data_end > int_reg_size_bytes) int_reg_size_bytes = (hUint32)data_end;
                }
            }
            floatRegCount = (float_reg_size_bytes+12)/(sizeof(float)*4);
            intRegCount = (int_reg_size_bytes+12)/(sizeof(hInt)*4);
            if (float_reg_size_bytes > 0) floatRegs.reset(new hFloat[float_reg_size_bytes/sizeof(float)]);
            if (int_reg_size_bytes > 0) intRegs.reset(new hInt[int_reg_size_bytes/sizeof(hInt)]);
        }

        hBool hasSampler(const hChar* name) {
            for (hUint32 i=0, n=header->constCount; i<n; ++i) {
                if (hStrCmp(name, strings+constants[i].stringTableOffset) == 0) {
                    return hTrue;
                }
            }
            return hFalse;
        }

        hBool getSamplerIndex(const hChar* name, hUint32* index) {
            hcAssert(index);
            for (hUint32 i=0, n=header->constCount; i<n; ++i) {
                if (hStrCmp(name, strings+constants[i].stringTableOffset) == 0) {
                    *index = constants[i].regStart;
                    return hTrue;
                }
            }
            return hFalse;
        }

        hBool getFloatParamStart(const hChar* name, hUint32* start, hUint32* bytesize) {
            hcAssert(start && bytesize);
            for (hUint32 i=0, n=header->constCount; i<n; ++i) {
                if (hStrCmp(name, strings+constants[i].stringTableOffset) == 0) {
                    *start = constants[i].regStart;
                    *bytesize = constants[i].bytes;
                    return hTrue;
                }
            }
            return hFalse;
        }

        hBool getIntParamStart(const hChar* name, hUint32* start, hUint32* bytesize) {
            hcAssert(start && bytesize);
            for (hUint32 i=0, n=header->constCount; i<n; ++i) {
                if (hStrCmp(name, strings+constants[i].stringTableOffset) == 0) {
                    *start = constants[i].regStart;
                    *bytesize = constants[i].bytes;
                    return hTrue;
                }
            }
            return hFalse;
        }

        hFloat* getFloatRegisterBaseAddress() {
            return floatRegs.get();
        }

        hInt* getIntRegisterBaseAddress() {
            return intRegs.get();
        }

        void uploadConstants(IDirect3DDevice9* d3d_device) {
            if (floatRegCount && vertexShader.get()) {
                d3d_device->SetVertexShaderConstantF(floatRegStart, floatRegs.get(), floatRegCount);
            }
            if (intRegCount && vertexShader.get()) {
                d3d_device->SetVertexShaderConstantI(intRegStart, intRegs.get(), intRegCount);
            }
            if (floatRegCount && pixelShader.get()) {
                d3d_device->SetPixelShaderConstantF(floatRegStart, floatRegs.get(), floatRegCount);
            }
            if (intRegCount && pixelShader.get()) {
                d3d_device->SetPixelShaderConstantI(intRegStart, intRegs.get(), intRegCount);
            }
        }

        hBool isBound() const { 
            return vertexShader.get() || pixelShader.get();
        }

        hBool bind(IDirect3DDevice9* d3d_device) {
            if (profile == hShaderProfile::D3D_9c_vs) {
                IDirect3DVertexShader9* vshader;
                auto hr = d3d_device->CreateVertexShader(programBlob, &vshader);
                if (hr != S_OK) {
                    hcAssertMsg(hr == S_OK, "CreateVertexShader() failed.");
                    return hFalse;
                }
                vertexShader.reset(vshader);
                return hTrue;
            } else if (profile == hShaderProfile::D3D_9c_ps) {
                IDirect3DPixelShader9* pshader;
                auto hr = d3d_device->CreatePixelShader(programBlob, &pshader);
                if (hr != S_OK) {
                    hcAssertMsg(hr == S_OK, "CreatePixelShader() failed.");
                    return hFalse;
                }
                pixelShader.reset(pshader);
                return hTrue;
            }

            return hFalse;
        }

        hShaderProfile profile;
        std::unique_ptr<hChar[]> shaderBlob;
        std::unique_ptr<hChar[]> shaderBlob2;
        const ReflectionHeader* header;
        const Constant* constants;
        const hChar* strings;
        const DWORD* programBlob;
        hUint32 floatRegStart, floatRegCount;
        hUint32 intRegStart, intRegCount;
        std::unique_ptr<hFloat[]> floatRegs;
        std::unique_ptr<hInt[]> intRegs;
        // No support for bools...yet
        IDirect3DVertexShader9Ptr vertexShader;
        IDirect3DPixelShader9Ptr pixelShader;
    };

    struct hTexture2D {
        hTexture2D(hUint32 w, hUint32 h, hMipDesc* in_mips, hUint32 mip_count, D3DFORMAT in_format) 
            : d3dFormat(in_format)
            , baseWidth(w)
            , baseHeight(h)
            , levels(mip_count)
            , usage(0) {
            for (hUint32 i=0; i < mip_count; ++i) {
                if (!in_mips[i].data) {
                    break;
                }
                mips.emplace_back(new hByte[in_mips[i].size]);
                hMemCpy(mips[i].get(), in_mips[i].data, in_mips[i].size);
            }
        }

        hBool isBound() const {
            return !!texture.get();
        }

        hBool bind(IDirect3DDevice9* d3d_device) {
            IDirect3DTexture9* tex;
            auto hr = d3d_device->CreateTexture(baseWidth, baseHeight, levels, usage, d3dFormat, D3DPOOL_MANAGED, &tex, nullptr);
            hcAssertMsg(hr == S_OK, "CreateTexture() failed.");
            if (hr != S_OK) return hFalse;
            texture.reset(tex);
            hUint lvl = 0;
            hUint32 h = baseHeight;
            hUint int_pitch = d3d9::getD3DFormatPitch(baseWidth, d3dFormat);

            for (const auto& mip : mips) {
                D3DLOCKED_RECT r;
                texture->LockRect(lvl, &r, nullptr, 0);
                hByte* src_ptr = mip.get();
                hByte* dest_ptr = (hByte*)r.pBits;
                for(hUint32 i=0; i<h; ++i) {
                    hMemCpy(dest_ptr, src_ptr, r.Pitch);
                    src_ptr+=int_pitch;
                    dest_ptr+=r.Pitch;
                }
                texture->UnlockRect(lvl);
                lvl++;
            }

            return hTrue;
        }

        D3DFORMAT d3dFormat;
        hUint32 baseWidth;
        hUint32 baseHeight;
        hUint32 levels;
        DWORD usage;
        std::vector<std::unique_ptr<hByte>> mips;
        IDirect3DTexture9Ptr texture;
    };

    struct hVertexBuffer {
        hVertexBuffer(hUint32 in_elementsize, hUint32 in_elementcount, const hByte* in_data)
            : elementSize(in_elementsize)
            , elementCount(in_elementcount)
            , usage(0)
            , data(new hByte[in_elementsize*in_elementcount]) {
            if (in_data) {
                hMemCpy(data.get(), in_data, in_elementsize*in_elementcount);
            }
        }

        hBool isBound() const {
            return !!vertexBuffer.get();
        }

        hBool bind(IDirect3DDevice9* d3d_device) {
            IDirect3DVertexBuffer9* vbuffer;
            auto hr = d3d_device->CreateVertexBuffer(elementSize*elementCount, usage, 0, D3DPOOL_MANAGED, &vbuffer, nullptr);
            hcAssertMsg(hr == S_OK, "CreateVertexBuffer() failed.");
            if (hr == S_OK) {
                vertexBuffer.reset(vbuffer);
                void* lock_ptr;
                vertexBuffer->Lock(0, 0, &lock_ptr, 0);
                hMemCpy(lock_ptr, data.get(), elementSize*elementCount);
                vertexBuffer->Unlock();
            }
            return hr == S_OK;
        }

        hUint32 elementSize;
        hUint32 elementCount;
        DWORD usage;
        std::unique_ptr<hByte> data;
        IDirect3DVertexBuffer9Ptr vertexBuffer;
    };

    struct hIndexBuffer {
        hIndexBuffer(hUint32 in_size, const hByte* in_data)
            : size(in_size)
            , usage(0)
            , data(new hByte[in_size]) {
            if (in_data) {
                hMemCpy(data.get(), in_data, in_size);
            }
        }

        hBool isBound() const {
            return !!indexBuffer.get();
        }

        hBool bind(IDirect3DDevice9* d3d_device) {
            IDirect3DIndexBuffer9* ibuffer;
            auto hr = d3d_device->CreateIndexBuffer(size, usage, D3DFMT_INDEX16, D3DPOOL_MANAGED, &ibuffer, nullptr);
            hcAssertMsg(hr == S_OK, "CreateVertexBuffer() failed.");
            if (hr == S_OK) {
                indexBuffer.reset(ibuffer);
                void* lock_ptr;
                indexBuffer->Lock(0, 0, &lock_ptr, 0);
                hMemCpy(lock_ptr, data.get(), size);
                indexBuffer->Unlock();
            }
            return hr == S_OK;
        }

        hUint32 size;
        DWORD usage;
        std::unique_ptr<hByte> data;
        IDirect3DIndexBuffer9Ptr indexBuffer;
    };

    struct hUniformBuffer {
        hUniformBuffer(hUint32 in_size, const hUniformLayoutDesc* in_layout, hUint in_count)
            : data(new hByte[in_size])
            , baseOffset(0)
            , size(in_size) {
            layout.resize(in_count);
            hMemCpy(layout.data(), in_layout, sizeof(hUniformLayoutDesc)*in_count);
        }

        hByte* getBase() const {
            return data.get()+baseOffset;
        }

        std::unique_ptr<hByte> data;
        std::vector<hUniformLayoutDesc> layout;
        hUint32 baseOffset;
        hUint32 size;
        
    };

    struct hSamplerState {
        /* -- Sampler states to update
            D3DSAMP_ADDRESSU;
            D3DSAMP_ADDRESSV;
            D3DSAMP_ADDRESSW;
            D3DSAMP_BORDERCOLOR;
            D3DSAMP_MAGFILTER;
            D3DSAMP_MINFILTER;
            D3DSAMP_MIPFILTER;
            D3DSAMP_MIPMAPLODBIAS;
            D3DSAMP_MAXMIPLEVEL;
            D3DSAMP_MAXANISOTROPY;
            //D3DSAMP_SRGBTEXTURE;
            //D3DSAMP_ELEMENTINDEX;
            //D3DSAMP_DMAPOFFSET;
        */
        struct StateValue {
            D3DSAMPLERSTATETYPE state;
            DWORD value;
        } states [9];
    };

    struct hRenderCall {
        /* -- Render States to update
            if (D3DRS_ALPHABLENDENABLE) { ->0
                D3DRS_SRCBLEND; ->1
                D3DRS_DESTBLEND; ->2
                D3DRS_BLENDOP; ->3
                if (D3DRS_SEPARATEALPHABLENDENABLE) { ->4
                    D3DRS_SRCBLENDALPHA; ->5
                    D3DRS_DESTBLENDALPHA; ->6
                    D3DRS_BLENDOPALPHA; ->7
                }
            }
            if (D3DRS_ZENABLE) { ->8
                D3DRS_ZWRITEENABLE; ->9
                D3DRS_ZFUNC; ->10
            }
            if (D3DRS_STENCILENABLE) { ->11
                D3DRS_STENCILFAIL; ->12
                D3DRS_STENCILZFAIL; ->13
                D3DRS_STENCILPASS; ->14
                D3DRS_STENCILFUNC; ->15
                D3DRS_STENCILREF; ->16
                D3DRS_STENCILMASK; ->17
                D3DRS_STENCILWRITEMASK; ->18
            }
            D3DRS_FILLMODE; ->19
            D3DRS_CULLMODE; ->20
            D3DRS_DEPTHBIAS; ->21
            D3DRS_SLOPESCALEDEPTHBIAS; ->22
            D3DRS_SCISSORTESTENABLE; ->23
        */
        struct RenderState {
            D3DRENDERSTATETYPE state;
            DWORD value;
        };
        struct SamplerState {
            hUint index;
            hTexture2D* texture;
            hSamplerState samplerState;
        };
        struct ParamUpdate {
            enum {
                Float = 1, 
                Int = 2, 
                Bool = 4,
                Vertex = 8,
                Fragment = 16,
            };
            hUint regIndex;
            hUint16 ubIndex;
            hUint16 ubOffset;
            hUint16 byteSize;
            hByte type;
        };

        hBool isBound() {
            return !!vtxDecl.get();
        }

        hBool bind(IDirect3DDevice9* d3d_device) {
            hBool bound = hTrue;
            if (!vertex->isBound()) bound &= vertex->bind(d3d_device);
            if (!pixel->isBound()) bound &= pixel->bind(d3d_device);
            if (ib && !ib->isBound()) bound &= ib->bind(d3d_device);
            if (!vb->isBound()) bound &= vb->bind(d3d_device);
            for (hUint i=0, n=samplerCount; i<n; ++i) {
                if (!samplerStates[i].texture->isBound()) bound &= samplerStates[i].texture->bind(d3d_device);            }
            if (bound && !vtxDecl.get()) {
                IDirect3DVertexDeclaration9* decl;
                auto hr = d3d_device->CreateVertexDeclaration(vtxDeclDesc, &decl);
                vtxDecl.reset(decl);
            }
            return bound && vtxDecl.get();
        }

        hShaderStage* vertex;
        hShaderStage* pixel;
        hVertexBuffer* vb;
        hIndexBuffer* ib;
        hUint stateCount;
        RenderState states[24];
        hUint samplerCount;
        SamplerState* samplerStates;
        hUint uniBufferCount;
        const hUniformBuffer** uniBuffers;
        hUint paramUpdateCount;
        ParamUpdate* paramUpdates;
        //hUint vtxDeclCount;
        D3DVERTEXELEMENT9* vtxDeclDesc;
        IDirect3DVertexDeclaration9Ptr vtxDecl;
    };

    struct hRenderFence {
        hRenderFence() {
            sema.Create(0, 1);
        }
        ~hRenderFence() {
            sema.Destroy();
        }
        void post() {
            sema.Post();
        }
        void wait() {
            sema.Wait();
        }

        lfds_freelist_element* element;
        hSemaphore sema;
    };

    enum class Op : hUint8 {
        NoOp = 0,
        Jump,
        Call,
        Return,
        CustomCall,
    };

    static hUint OpCodeSize = 8; // required for cmd alignment, 4 for opcode, 4 for next cmd offset

    struct hRndrOpJump {
        void* next;
    };

    struct hRndrOpCall {
        hCmdList* jumpTo;
    };

    /*-- The following is intended as a quick and dirty start up for adding new render calls. */
    struct hRndrOpCustomCallBase {
        virtual hUint execute() = 0;
    };

    template<typename t_ty>
    struct hRndrOpCustomCall : hRndrOpCustomCallBase{
        hRndrOpCustomCall(t_ty in_fn)
            : fn(in_fn) {}
        hUint execute() override {
            return fn();
        }

        t_ty fn;
    };
    /*-- End quick and dirty start up for adding new render calls. */

    struct hCmdList {
        static const hUint MinCmdBlockSize = 4 * 1024;
        hCmdList() {
            prev = next = this;
            cmds = nullptr;
            nextcmd = nullptr;
            cmdsSize = 0;
            cmdsReserve = 0;
        }

        hByte* allocCmdMem(Op opcode, hUint s) {
            auto needed = s + OpCodeSize;

            if ((cmdsReserve - cmdsSize) < needed) {
                hByte* memnext = (hByte*)rtmp_malloc(MinCmdBlockSize);
                if (nextcmd != nullptr) {
                    // need to write a jump
                    auto j = nextcmd + cmdsSize;
                    cmdsSize += sizeof(hRndrOpJump) + OpCodeSize;
                    *((Op*)j) = Op::Jump;
                    *((hUint32*)(j+4)) = 0;
                    ((hRndrOpJump*)(j + OpCodeSize))->next = memnext;
                }
                nextcmd = memnext;
                // note remaining space but leave enough for another jump cmd
                cmdsReserve += MinCmdBlockSize - (sizeof(hRndrOpJump) + OpCodeSize);
                if (!cmds) {
                    cmds = nextcmd;
                }
            }

            hByte* r = nextcmd;
            cmdsSize += needed; nextcmd += needed;

            *((Op*)r) = opcode;
            *((hUint32*)(r+4)) = s;
            return (r + OpCodeSize);
        }

        hCmdList* prev, *next;
        hByte* cmds, *nextcmd;
        hUint  cmdsSize;
        hUint  cmdsReserve;
    };
namespace d3d9 {
    void renderDoFrame();
    hBool isRenderThread() {
        return Device::GetCurrentThreadID() == rtID;
    }

    struct hRenderDestructBase {
        hRenderDestructBase()  {
            next = prev = this;
        }
        virtual ~hRenderDestructBase() {
        }

        hRenderDestructBase* next, *prev;

        static void linkLists(hRenderDestructBase* before, hRenderDestructBase* after, hRenderDestructBase* i) {
            i->next = before->next;
            i->prev = after->prev;
            before->next = i;
            after->prev = i;
        }

        static void detachLists(hRenderDestructBase* i) {
            i->next->prev = i->prev;
            i->prev->next = i->next;
            i->next = i;
            i->prev = i;
        }
    };

    template<typename t_ty>
    struct hRenderDestruct : public hRenderDestructBase {
        hRenderDestruct(t_ty& t) : proc(t) {}
        ~hRenderDestruct() {
            proc();
        }
        t_ty proc;
    };

    hUint32             rtFrameIdx;
    hRenderDestructBase pendingDeletes[FRAME_COUNT];

    hRenderDestructBase* dequeueRenderResourceDelete() {
        lfds_queue_use(destructionQueue);
        hRenderDestructBase* ret = nullptr;
        lfds_queue_dequeue(destructionQueue, (void**)&ret);
        return ret;
    }

    template < typename t_ty >
    static void enqueueRenderResourceDelete(t_ty fn) {
        auto* r = new hRenderDestruct<t_ty>(fn);
        lfds_queue_use(destructionQueue);
        while (lfds_queue_enqueue(destructionQueue, r) == 0) {
            Device::ThreadYield();
        }
    }

    void initD3D9(HWND hwnd) {
        if ((pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        {
            hcAssertMsg(pD3D, "Failed to create D3D");
            return;
        }
        D3DPRESENT_PARAMETERS    d3dpp;
        ZeroMemory(&d3dpp, sizeof(d3dpp));
        d3dpp.Windowed = TRUE;
        d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
        d3dpp.EnableAutoDepthStencil = TRUE;
        d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
        d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

        // Create the D3DDevice
        if (pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &d3dpp, &d3dDevice) < 0)
        {
            hcAssertFailMsg("Failed to create D3D Device");
            return;
        }

        rtID = Device::GetCurrentThreadID();

        d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

        d3dFormatPitch[D3DFMT_R8G8B8] = 4;
        d3dFormatPitch[D3DFMT_A8R8G8B8] = 4;
        d3dFormatPitch[D3DFMT_X8R8G8B8] = 4;
        d3dFormatPitch[D3DFMT_R5G6B5] = 2;
        d3dFormatPitch[D3DFMT_X1R5G5B5] = 2;
        d3dFormatPitch[D3DFMT_A1R5G5B5] = 2;
        d3dFormatPitch[D3DFMT_A4R4G4B4] = 2;
        d3dFormatPitch[D3DFMT_R3G3B2] = 1;
        d3dFormatPitch[D3DFMT_A8] = 1;
        d3dFormatPitch[D3DFMT_A8R3G3B2] = 2;
        d3dFormatPitch[D3DFMT_X4R4G4B4] = 2;
        d3dFormatPitch[D3DFMT_A2B10G10R10] = 4;
        d3dFormatPitch[D3DFMT_A8B8G8R8] = 4;
        d3dFormatPitch[D3DFMT_X8B8G8R8] = 4;
        d3dFormatPitch[D3DFMT_G16R16] = 4;
        d3dFormatPitch[D3DFMT_A2R10G10B10] = 4;
        d3dFormatPitch[D3DFMT_A16B16G16R16] = 8;
        d3dFormatPitch[D3DFMT_A8P8] = 2;
        d3dFormatPitch[D3DFMT_P8] = 1;
        d3dFormatPitch[D3DFMT_L8] = 1;
        d3dFormatPitch[D3DFMT_A8L8] = 2;
        d3dFormatPitch[D3DFMT_A4L4] = 2;
        d3dFormatPitch[D3DFMT_V8U8] = 2;
        d3dFormatPitch[D3DFMT_L6V5U5] = 2;
        d3dFormatPitch[D3DFMT_X8L8V8U8] = 4;
        d3dFormatPitch[D3DFMT_Q8W8V8U8] = 4;
        d3dFormatPitch[D3DFMT_V16U16] = 2;
        d3dFormatPitch[D3DFMT_A2W10V10U10] = 4;
        d3dFormatPitch[D3DFMT_UYVY] = 4;
        d3dFormatPitch[D3DFMT_R8G8_B8G8] = 4;
        d3dFormatPitch[D3DFMT_YUY2] = 4;
        d3dFormatPitch[D3DFMT_G8R8_G8B8] = 4;
        d3dFormatPitch[D3DFMT_DXT1] = BC_PITCH;
        d3dFormatPitch[D3DFMT_DXT2] = BC_PITCH;
        d3dFormatPitch[D3DFMT_DXT3] = BC_PITCH;
        d3dFormatPitch[D3DFMT_DXT4] = BC_PITCH;
        d3dFormatPitch[D3DFMT_DXT5] = BC_PITCH;
        d3dFormatPitch[D3DFMT_D16_LOCKABLE] = 2;
        d3dFormatPitch[D3DFMT_D32] = 4;
        d3dFormatPitch[D3DFMT_D15S1] = 2;
        d3dFormatPitch[D3DFMT_D24S8] = 4;
        d3dFormatPitch[D3DFMT_D24X8] = 4;
        d3dFormatPitch[D3DFMT_D24X4S4] = 4;
        d3dFormatPitch[D3DFMT_D16] = 2;
        d3dFormatPitch[D3DFMT_D32F_LOCKABLE] = 4;
        d3dFormatPitch[D3DFMT_D24FS8] = 4;
        d3dFormatPitch[D3DFMT_L16] = 2;
        d3dFormatPitch[D3DFMT_VERTEXDATA] = 0;
        d3dFormatPitch[D3DFMT_INDEX16] = 0;
        d3dFormatPitch[D3DFMT_INDEX32] = 0;
        d3dFormatPitch[D3DFMT_Q16W16V16U16] = 8;
        d3dFormatPitch[D3DFMT_MULTI2_ARGB8] = 0;
        d3dFormatPitch[D3DFMT_R16F] = 2;
        d3dFormatPitch[D3DFMT_G16R16F] = 4;
        d3dFormatPitch[D3DFMT_A16B16G16R16F] = 8;
        d3dFormatPitch[D3DFMT_R32F] = 4;
        d3dFormatPitch[D3DFMT_G32R32F] = 8;
        d3dFormatPitch[D3DFMT_A32B32G32R32F] = 16;
        d3dFormatPitch[D3DFMT_CxV8U8] = 2;
#if !defined(D3D_DISABLE_9EX)
        //D3DFMT_D32_LOCKABLE         = 84,
        //D3DFMT_S8_LOCKABLE          = 85,
        //D3DFMT_A1                   = 118,
        //D3DFMT_A2B10G10R10_XR_BIAS  = 119,
        //D3DFMT_BINARYBUFFER         = 199,
#endif // !D3D_DISABLE_9EX
    }

    hUint32 getBCPitch(hBool dxt1, hUint32 width) {
        hUint32 bytecount = hMax( 1u, ((width+3)/4) );
        bytecount *= (dxt1) ? 8 : 16;
        return bytecount;
    }

    hUint getD3DFormatPitch(hUint32 width, D3DFORMAT fmt) {
        return (d3dFormatPitch[fmt] == BC_PITCH) ? getBCPitch(fmt == D3DFMT_DXT1, width) : d3dFormatPitch[fmt]*width;
    }

    hUint32 renderThreadMain(void* param) {
        initD3D9((HWND)param);

        rtComsSignal.signal();

        int frame = 0;
        for (;;) {
            renderDoFrame();
            ++frame;
        }

        return 0;
    }

    void create(hSystem* system, hUint32 width, hUint32 height, hUint32 bpp, hFloat shaderVersion, hBool fullscreen, hBool vsync) {

    	multiThreadedRenderer = !!configVars->getCVarUint("renderer.gl.multithreaded", 1);
        renderScratchMemSize = configVars->getCVarUint("renderer.scratchmemsize", 1*1024*1024);
    	fenceCount = configVars->getCVarUint("renderer.fencecount", 256);
        auto destruction_queue_size = configVars->getCVarUint("renderer.destroyqueuesize", 256);
        auto cmd_queue_size = configVars->getCVarUint("renderer.cmdlistqueuesize", 256);
        renderCommandThreshold = configVars->getCVarUint("renderer.commandthreshold", 32);
        
    	fences = new hRenderFence[fenceCount];

    	lfds_freelist_new(&fenceFreeList, fenceCount, [](void** user_data, void* user_state) -> int {
    		static hUint initFaces = 0;
    		auto* fences = (hRenderFence*)user_state;
    		*user_data = fences+initFaces;
    		++initFaces;
    		return 1;
    	}, fences);
    	
    	fenceIndex = 0;
    	for (auto& i:frameFences) {
    		i = nullptr;
    	}
        
        lfds_queue_new(&cmdListQueue, cmd_queue_size);
        lfds_queue_new(&destructionQueue, destruction_queue_size);
        
        for (auto& mem : renderScratchMem) {
            mem = (hByte*)hMalloc(renderScratchMemSize);
        }

        SDL_SysWMinfo info;
        SDL_VERSION(&info.version);
        auto* window = system->getSDLWindow();
        if(SDL_GetWindowWMInfo(window,&info)) {

        }
        HWND hwnd = info.info.win.window;
        

    	if (multiThreadedRenderer == true) {
    		renderThread.create("D3D9 Render Thread", hThread::PRIORITY_NORMAL, hFUNCTOR_BINDSTATIC(hThreadFunc, renderThreadMain), (void*)hwnd);

    		rtMutex.Lock();
    		rtComsSignal.wait(&rtMutex);
    		rtMutex.Unlock();
    	} else {
            initD3D9(hwnd);
        }
    }

    void destroy() {

    }

    bool isProfileSupported(hShaderProfile profile) {
        return (profile == hShaderProfile::D3D_9c_vs || profile == hShaderProfile::D3D_9c_ps);
    }

    hShaderProfile getActiveProfile(hShaderFrequency freq) {
        if (freq == hShaderFrequency::Vertex) return hShaderProfile::D3D_9c_vs;
        if (freq == hShaderFrequency::Pixel) return hShaderProfile::D3D_9c_ps;
        return hShaderProfile::Invalid;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    const hRenderFrameStats* getRenderStats()  {
        return nullptr;
    }
    
    hShaderStage* compileShaderStageFromSource(const hChar* shaderProg, hUint32 len, const hChar* entry, hShaderProfile profile) {
        return new hShaderStage(profile, shaderProg, len);
    }
    
    void  destroyShader(hShaderStage* prog) {
        enqueueRenderResourceDelete([=](){
            delete prog;
        });
    }

    hTexture2D*  createTexture2D(hUint32 levels, hMipDesc* initialData, hTextureFormat format, hUint32 flags) {
        bool compressed = true;
        D3DFORMAT intfmt;
        switch(format) {
        case hTextureFormat::R8_unorm:         intfmt=D3DFMT_L8; compressed = false; break; 
        case hTextureFormat::RGBA8_unorm:      intfmt=D3DFMT_A8R8G8B8; compressed = false; break; 
        case hTextureFormat::RGBA8_sRGB_unorm: intfmt=D3DFMT_A8R8G8B8; compressed = false; break; 
        case hTextureFormat::BC1_unorm:        intfmt=D3DFMT_DXT1; compressed = true;  break;
        case hTextureFormat::BC2_unorm:        intfmt=D3DFMT_DXT3; compressed = true;  break;
        case hTextureFormat::BC3_unorm:        intfmt=D3DFMT_DXT5; compressed = true;  break;
        case hTextureFormat::BC1_sRGB_unorm:   intfmt=D3DFMT_DXT1; compressed = true;  break;
        case hTextureFormat::BC2_sRGB_unorm:   intfmt=D3DFMT_DXT3; compressed = true;  break;
        case hTextureFormat::BC3_sRGB_unorm:   intfmt=D3DFMT_DXT5; compressed = true;  break;
        case hTextureFormat::BC4_unorm:
        case hTextureFormat::BC5_unorm:
        default: hcAssertFailMsg("Can't handle texture format"); return nullptr;
        }
        return new hTexture2D(initialData->width, initialData->height, initialData, levels, intfmt);
    }
    void  destroyTexture2D(hTexture2D* t) {
        enqueueRenderResourceDelete([=](){
            delete t;
        });
    }

    hIndexBuffer* createIndexBuffer(const void* pIndices, hUint32 nIndices, hUint32 flags) {
        return new hIndexBuffer(nIndices*sizeof(hUint16), (const hByte*)pIndices);
    }
    void* getIndexBufferMappingPtr(hIndexBuffer* vb) {
        return vb->data.get();
    }
    void  destroyIndexBuffer(hIndexBuffer* ib) {
        enqueueRenderResourceDelete([=]() {
            delete ib;
        });
    }

    hVertexBuffer*  createVertexBuffer(const void* initData, hUint32 elementsize, hUint32 elementcount, hUint32 flags) {
        return new hVertexBuffer(elementsize, elementcount, (const hByte*)initData);
    }
    void* getVertexBufferMappingPtr(hVertexBuffer* vb, hUint32* size) {
        return vb->data.get();
    }
    void  destroyVertexBuffer(hVertexBuffer* vb) {
        enqueueRenderResourceDelete([=]() {
            delete vb;
        });
    }

    hUniformBuffer* createUniformBuffer(const void* initdata, const hUniformLayoutDesc* layout, hUint layout_count, hUint structSize, hUint bufferCount, hUint32 flags) {
        return new hUniformBuffer(structSize*bufferCount, layout, layout_count);
    }
    const hUniformLayoutDesc* getUniformBufferLayoutInfo(const hUniformBuffer* ub, hUint* out_count) {
        hcAssert(out_count);
        *out_count = (hUint)ub->layout.size();
        return ub->layout.data();
    }
    void* getUniformBufferMappingPtr(hUniformBuffer* ub) {
        return ub->data.get();
    }
    void destroyUniformBuffer(hUniformBuffer* ub) {
        enqueueRenderResourceDelete([=](){
            delete ub;
        });
    }

    hRenderCall* createRenderCall(const hRenderCallDescBase& rcd) {
        static const hUint sampler_size = sizeof(hRenderCall::SamplerState);
        hUint sampler_count = 0;
        hUint int_param_update_count = 0;
        hUint float_param_update_count = 0;
        hUint uniform_buffer_count = 0;

        for (hUint i = 0; i<rcd.vertex_->header->constCount; ++i) {
            const auto& sampler = rcd.vertex_->constants[i];
            if (sampler.type == 3) {
                ++sampler_count;
            }
        }
        for (hUint i = 0; i<rcd.fragment_->header->constCount; ++i) {
            const auto& sampler = rcd.fragment_->constants[i];
            if (sampler.type == 3) {
                ++sampler_count;
            }
        }
        for (hUint i = 0; i<rcd.vertex_->header->constCount; ++i) {
            const auto& const_param = rcd.vertex_->constants[i];
            if (const_param.type == 1) {
                ++int_param_update_count;
            } else if (const_param.type == 2) {
                ++float_param_update_count;
            }
        }
        for (hUint i = 0; i<rcd.fragment_->header->constCount; ++i) {
            const auto& const_param = rcd.fragment_->constants[i];
            if (const_param.type == 1) {
                ++int_param_update_count;
            } else if (const_param.type == 2) {
                ++float_param_update_count;
            }
        }
        for (const auto& uni_buffer : rcd.uniformBuffers_) {
            if (!uni_buffer.name_.is_default()) {
                ++uniform_buffer_count;
            }
        }

        auto blendOpConv = [](proto::renderstate::BlendOp in) -> DWORD {
            switch(in) {
            case proto::renderstate::BlendZero             : return D3DBLEND_ZERO;
            case proto::renderstate::BlendOne              : return D3DBLEND_ONE;
            case proto::renderstate::BlendSrcColour        : return D3DBLEND_SRCCOLOR;
            case proto::renderstate::BlendInverseSrcColour : return D3DBLEND_INVSRCCOLOR;
            case proto::renderstate::BlendDestColour       : return D3DBLEND_DESTCOLOR;
            case proto::renderstate::BlendInverseDestColour: return D3DBLEND_INVDESTCOLOR;
            case proto::renderstate::BlendSrcAlpha         : return D3DBLEND_SRCALPHA;
            case proto::renderstate::BlendInverseSrcAlpha  : return D3DBLEND_INVSRCALPHA;
            case proto::renderstate::BlendDestAlpha        : return D3DBLEND_DESTALPHA;
            case proto::renderstate::BlendInverseDestAlpha : return D3DBLEND_INVDESTALPHA;
            }
            return D3DBLEND_ZERO;
        };
        auto blendFuncConv = [](proto::renderstate::BlendFunction in) -> DWORD {
            switch (in) {
            case proto::renderstate::Add: return D3DBLENDOP_ADD;
            case proto::renderstate::Sub: return D3DBLENDOP_SUBTRACT;
            case proto::renderstate::Min: return D3DBLENDOP_MIN;
            case proto::renderstate::Max: return D3DBLENDOP_MAX;
            }
            return D3DBLENDOP_ADD;
        };
        auto funcCompConv = [](proto::renderstate::FunctionCompare in) -> DWORD {
            switch(in) {
            case proto::renderstate::CompareNever        : return D3DCMP_NEVER;
            case proto::renderstate::CompareLess         : return D3DCMP_LESS;
            case proto::renderstate::CompareEqual        : return D3DCMP_EQUAL;
            case proto::renderstate::CompareLessEqual    : return D3DCMP_LESSEQUAL;
            case proto::renderstate::CompareGreater      : return D3DCMP_GREATER;
            case proto::renderstate::CompareNotEqual     : return D3DCMP_NOTEQUAL;
            case proto::renderstate::CompareGreaterEqual : return D3DCMP_GREATEREQUAL;
            case proto::renderstate::CompareAlways       : return D3DCMP_ALWAYS;
            }
            return D3DCMP_NEVER;
        };
        auto stencilOpConv = [](proto::renderstate::StencilOp in) -> DWORD {
            switch(in) {
            case proto::renderstate::StencilKeep   : return D3DSTENCILOP_KEEP;
            case proto::renderstate::StencilZero   : return D3DSTENCILOP_ZERO;
            case proto::renderstate::StencilReplace: return D3DSTENCILOP_REPLACE;
            case proto::renderstate::StencilIncSat : return D3DSTENCILOP_INCRSAT;
            case proto::renderstate::StencilDecSat : return D3DSTENCILOP_DECRSAT;
            case proto::renderstate::StencilInvert : return D3DSTENCILOP_INVERT;
            case proto::renderstate::StencilIncr   : return D3DSTENCILOP_INCR;
            case proto::renderstate::StencilDecr   : return D3DSTENCILOP_DECR;
            }
            return D3DSTENCILOP_KEEP;
        };
        auto fillModeConv = [](proto::renderstate::FillMode in) -> DWORD {
            if (in == proto::renderstate::Wireframe) {
                return D3DFILL_WIREFRAME;
            } else {
                return D3DFILL_SOLID;
            }
        };
        auto cullModeConv = [](proto::renderstate::CullMode in) -> DWORD {
            switch(in) {
            case proto::renderstate::CullNone            : return D3DCULL_NONE;
            case proto::renderstate::CullClockwise       : return D3DCULL_CW;
            case proto::renderstate::CullCounterClockwise: return D3DCULL_CCW;
            }
            return D3DCULL_CW;
        };
        auto textureFilterConv = [](proto::renderstate::SamplerState in) {
            switch(in) {
            case proto::renderstate::point      : return D3DTEXF_POINT;
            case proto::renderstate::linear     : return D3DTEXF_LINEAR;
            case proto::renderstate::anisotropic: return D3DTEXF_ANISOTROPIC;
            }
            return D3DTEXF_POINT;
        };
        auto textureAddConv = [](proto::renderstate::SamplerBorder in) {
            switch(in) {
            case proto::renderstate::wrap  : return D3DTADDRESS_WRAP;
            case proto::renderstate::mirror: return D3DTADDRESS_MIRROR;
            case proto::renderstate::clamp : return D3DTADDRESS_CLAMP;
            case proto::renderstate::border: return D3DTADDRESS_BORDER;
            }
            return D3DTADDRESS_CLAMP;
        };
        auto floatBitsToDWORD = [](float in) {
            union {
                float f;
                DWORD d;
            } mish_mash;
            mish_mash.f = in;
            return mish_mash.d;
        };
        auto intBitsToDWORD = [](hInt32 in) {
            union {
                hInt32 i;
                DWORD d;
            } mish_mash;
            mish_mash.i = in;
            return mish_mash.d;
        };

        bool separate_alpha_blend =
        rcd.blend_.blendEnable_ && 
        (rcd.blend_.srcBlend_ != rcd.blend_.srcBlendAlpha_ ||
        rcd.blend_.destBlend_ != rcd.blend_.destBlendAlpha_ ||
        rcd.blend_.blendOp_ != rcd.blend_.blendOpAlpha_);

        hSize_t rnd_call_size = sizeof(hRenderCall) 
            + (sampler_count*sampler_size) 
            + (int_param_update_count*sizeof(hRenderCall::ParamUpdate))
            + (float_param_update_count*sizeof(hRenderCall::ParamUpdate))
            + (uniform_buffer_count*sizeof(void*))
            + ((rcd.vertexLayoutCount+1)*sizeof(D3DVERTEXELEMENT9));
        auto* ptr = (hByte*)hMalloc(rnd_call_size*2);
        auto* rc = new (ptr) hRenderCall();
        rc->samplerStates = (hRenderCall::SamplerState*)(ptr+sizeof(hRenderCall));
        rc->samplerCount = sampler_count;
        rc->uniBuffers = (const hUniformBuffer**)(ptr+(sizeof(hRenderCall)+(sampler_count*sampler_size)));
        rc->uniBufferCount = uniform_buffer_count;
        rc->paramUpdates = (hRenderCall::ParamUpdate*)(ptr+(sizeof(hRenderCall)+(sampler_count*sampler_size)+(uniform_buffer_count*sizeof(void*))));
        rc->paramUpdateCount = int_param_update_count+float_param_update_count;
        rc->vtxDeclDesc = (D3DVERTEXELEMENT9*)(ptr+(sizeof(hRenderCall)+(sampler_count*sampler_size)+(uniform_buffer_count*sizeof(void*))+(rc->paramUpdateCount*sizeof(hRenderCall::ParamUpdate))));
        rc->vertex=rcd.vertex_;
        rc->pixel=rcd.fragment_;
        rc->vb = rcd.vertexBuffer_;
        rc->ib = rcd.indexBuffer_;
        rc->states[0]  = { D3DRS_ALPHABLENDENABLE, (DWORD)(rcd.blend_.blendEnable_ ? TRUE : FALSE) };
        rc->states[1]  = { D3DRS_SRCBLEND, blendOpConv(rcd.blend_.srcBlend_) };
        rc->states[2]  = { D3DRS_DESTBLEND, blendOpConv(rcd.blend_.destBlend_) };
        rc->states[3]  = { D3DRS_BLENDOP, blendFuncConv(rcd.blend_.blendOp_) };
        rc->states[4]  = { D3DRS_SEPARATEALPHABLENDENABLE, (DWORD)(separate_alpha_blend ? FALSE : FALSE) };
        rc->states[5]  = { D3DRS_SRCBLENDALPHA, blendOpConv(rcd.blend_.srcBlendAlpha_) };
        rc->states[6]  = { D3DRS_DESTBLENDALPHA, blendOpConv(rcd.blend_.destBlendAlpha_) };
        rc->states[7]  = { D3DRS_BLENDOPALPHA, blendFuncConv(rcd.blend_.blendOpAlpha_) };
        rc->states[8]  = { D3DRS_ZENABLE, (DWORD)(rcd.depthStencil_.depthEnable_ ? TRUE : FALSE) };
        rc->states[9]  = { D3DRS_ZWRITEENABLE, (DWORD)(rcd.depthStencil_.depthEnable_ ? TRUE : FALSE) };
        rc->states[10] = { D3DRS_ZFUNC, funcCompConv(rcd.depthStencil_.depthFunc_) };
        rc->states[11] = { D3DRS_STENCILENABLE, rcd.depthStencil_.stencilEnable_ };
        rc->states[12] = { D3DRS_STENCILFAIL, stencilOpConv(rcd.depthStencil_.stencilFailOp_) };
        rc->states[13] = { D3DRS_STENCILZFAIL, stencilOpConv(rcd.depthStencil_.stencilDepthFailOp_) };
        rc->states[14] = { D3DRS_STENCILPASS, stencilOpConv(rcd.depthStencil_.stencilPassOp_) };
        rc->states[15] = { D3DRS_STENCILFUNC, funcCompConv(rcd.depthStencil_.stencilFunc_) };
        rc->states[16] = { D3DRS_STENCILREF, rcd.depthStencil_.stencilRef_ };
        rc->states[17] = { D3DRS_STENCILMASK, rcd.depthStencil_.stencilReadMask_ };
        rc->states[18] = { D3DRS_STENCILWRITEMASK, rcd.depthStencil_.stencilWriteMask_ };
        rc->states[19] = { D3DRS_FILLMODE, fillModeConv(rcd.rasterizer_.fillMode_) };
        rc->states[20] = { D3DRS_CULLMODE, cullModeConv(rcd.rasterizer_.cullMode_) };
        rc->states[21] = { D3DRS_DEPTHBIAS, intBitsToDWORD(rcd.rasterizer_.depthBias_) };
        rc->states[22] = { D3DRS_SLOPESCALEDEPTHBIAS, floatBitsToDWORD(rcd.rasterizer_.slopeScaledDepthBias_) };
        rc->states[23] = { D3DRS_SCISSORTESTENABLE, (DWORD)(rcd.rasterizer_.scissorEnable_ ? TRUE : FALSE) };

        auto addSampler = [&](hRenderCall* in_rc, hUint i, hUint sampler_reg, const hRenderCallDesc::hSamplerStateDesc& ss, hTexture2D* in_tex) {
            hcAssertMsg(in_tex, "No texture bound to sampler.");
            in_rc->samplerStates[i].index = sampler_reg;
            in_rc->samplerStates[i].texture = in_tex;
            in_rc->samplerStates[i].samplerState.states[0] = { D3DSAMP_ADDRESSU, (DWORD)textureAddConv(ss.addressU_)};
            in_rc->samplerStates[i].samplerState.states[1] = { D3DSAMP_ADDRESSV, (DWORD)textureAddConv(ss.addressV_)};
            in_rc->samplerStates[i].samplerState.states[2] = { D3DSAMP_ADDRESSW, (DWORD)textureAddConv(ss.addressW_)};
            in_rc->samplerStates[i].samplerState.states[3] = { D3DSAMP_BORDERCOLOR, (hUint32)ss.borderColour_};
            in_rc->samplerStates[i].samplerState.states[4] = { D3DSAMP_MAGFILTER, (DWORD)textureFilterConv(ss.filter_)};
            in_rc->samplerStates[i].samplerState.states[5] = { D3DSAMP_MINFILTER, (DWORD)textureFilterConv(ss.filter_)};
            in_rc->samplerStates[i].samplerState.states[6] = { D3DSAMP_MAXANISOTROPY, ss.maxAnisotropy_};
            in_rc->samplerStates[i].samplerState.states[7] = { D3DSAMP_MAXMIPLEVEL, floatBitsToDWORD(ss.maxLOD_)};
            in_rc->samplerStates[i].samplerState.states[8] = { D3DSAMP_MIPMAPLODBIAS, floatBitsToDWORD(ss.mipLODBias_)};
        };
        auto findMatchingSampler = [](const hRenderCallDescBase& in_rcd, const hChar* name) -> const hRenderCallDesc::hSamplerStateDesc* {
            for (const auto& i : in_rcd.samplerStates_) {
                if (!i.name_.is_default() && hStrCmp(i.name_.c_str(), name) == 0) {
                    return &i.sampler_;
                }
            }
            return nullptr;
        };
        auto findMatchingTexture = [](const hRenderCallDescBase& in_rcd, const hChar* name) -> hTexture2D* {
            for (const auto& i : in_rcd.textureSlots_) {
                if (!i.name_.is_default() && hStrCmp(i.name_.c_str(), name) == 0) {
                    return i.t2D_;
                }
            }
            return nullptr;
        };
        hUint current_sampler = 0;
        for (hUint i = 0; i<rcd.vertex_->header->constCount; ++i) {
            const auto& sampler = rcd.vertex_->constants[i];
            if (sampler.type != 3) continue;
            if (auto* ss_ptr = findMatchingSampler(rcd, rcd.vertex_->strings+sampler.stringTableOffset)) {
                addSampler(rc, current_sampler++, sampler.regStart, *ss_ptr, findMatchingTexture(rcd, rcd.vertex_->strings+sampler.stringTableOffset));
            }
        }
        for (hUint i = 0; i<rcd.fragment_->header->constCount; ++i) {
            const auto& sampler = rcd.fragment_->constants[i];
            if (sampler.type != 3) continue;
            if (auto* ss_ptr = findMatchingSampler(rcd, rcd.fragment_->strings+sampler.stringTableOffset)) {
                addSampler(rc, current_sampler++, sampler.regStart, *ss_ptr, findMatchingTexture(rcd, rcd.fragment_->strings+sampler.stringTableOffset));
            }
        }
        hcAssert(current_sampler == rc->samplerCount);

        hUint current_ub = 0;
        for (auto& uni_buffer : rcd.uniformBuffers_) {
            if (!uni_buffer.name_.is_default()) {
                rc->uniBuffers[current_ub++] = uni_buffer.ub_;
            }
        }
        hcAssert(current_ub == rc->uniBufferCount);

        hUint current_pu = 0;
        for (hUint i = 0; i<rcd.vertex_->header->constCount; ++i) {
            const auto& const_param = rcd.vertex_->constants[i];
            hUint index, offset, size;
            ShaderParamType type;
            if (const_param.type == 1 && rcd.findNamedParameter(rcd.vertex_->strings+const_param.stringTableOffset, &index, &offset, &size, &type)) {
                rc->paramUpdates[current_pu].regIndex = const_param.regStart;
                rc->paramUpdates[current_pu].ubIndex = index;
                rc->paramUpdates[current_pu].ubOffset = offset;
                rc->paramUpdates[current_pu].byteSize = size;
                rc->paramUpdates[current_pu].type = hRenderCall::ParamUpdate::Vertex | hRenderCall::ParamUpdate::Int;
                ++current_pu;
            } else if (const_param.type == 2 && rcd.findNamedParameter(rcd.vertex_->strings+const_param.stringTableOffset, &index, &offset, &size, &type)) {
                rc->paramUpdates[current_pu].regIndex = const_param.regStart;
                rc->paramUpdates[current_pu].ubIndex = index;
                rc->paramUpdates[current_pu].ubOffset = offset;
                rc->paramUpdates[current_pu].byteSize = size;
                rc->paramUpdates[current_pu].type = hRenderCall::ParamUpdate::Vertex | hRenderCall::ParamUpdate::Float;
                ++current_pu;
            }
        }
        for (hUint i = 0; i<rcd.fragment_->header->constCount; ++i) {
            const auto& const_param = rcd.fragment_->constants[i];
            hUint index, offset, size;
            ShaderParamType type;
            if (const_param.type == 1 && rcd.findNamedParameter(rcd.fragment_->strings+const_param.stringTableOffset, &index, &offset, &size, &type)) {
                rc->paramUpdates[current_pu].regIndex = const_param.regStart;
                rc->paramUpdates[current_pu].ubIndex = index;
                rc->paramUpdates[current_pu].ubOffset = offset;
                rc->paramUpdates[current_pu].byteSize = size;
                rc->paramUpdates[current_pu].type = hRenderCall::ParamUpdate::Fragment | hRenderCall::ParamUpdate::Int;
                ++current_pu;
            } else if (const_param.type == 2 && rcd.findNamedParameter(rcd.fragment_->strings+const_param.stringTableOffset, &index, &offset, &size, &type)) {
                rc->paramUpdates[current_pu].regIndex = const_param.regStart;
                rc->paramUpdates[current_pu].ubIndex = index;
                rc->paramUpdates[current_pu].ubOffset = offset;
                rc->paramUpdates[current_pu].byteSize = size;
                rc->paramUpdates[current_pu].type = hRenderCall::ParamUpdate::Fragment | hRenderCall::ParamUpdate::Float;
                ++current_pu;
            }
        }
        hcAssert(current_pu == rc->paramUpdateCount);

        auto vtxDeclTypeConv = [](hVertexInputType in, hUint count, hBool normalise) {
            switch(in) {
            case hVertexInputType::Byte: hcAssert(count == 4 && !normalise); return D3DDECLTYPE_UBYTE4;
            case hVertexInputType::UByte: hcAssert(count == 4); return normalise ? D3DDECLTYPE_D3DCOLOR : D3DDECLTYPE_UBYTE4;
            case hVertexInputType::Short: {
                hcAssert(count == 4 || count == 2); 
                return (D3DDECLTYPE)((normalise ? D3DDECLTYPE_SHORT2N : D3DDECLTYPE_SHORT2) + (count-2));
            }
            case hVertexInputType::UShort: {
                hcAssert(count == 4 || count == 2);
                hcAssert(normalise);
                return (D3DDECLTYPE)(D3DDECLTYPE_USHORT2N + (count-2));
            }
            //case hVertexInputType::Int: return ;
            //case hVertexInputType::UInt: return ;
            case hVertexInputType::HalfFloat: {
                return (D3DDECLTYPE)(D3DDECLTYPE_FLOAT16_2 + (count-2));
            }
            case hVertexInputType::Float: return (D3DDECLTYPE)(D3DDECLTYPE_FLOAT1 + (count-1));
            //case hVertexInputType::Double: return ;
            }
            hcAssertFailMsg("Unsupported vertex format!");
            return D3DDECLTYPE_UNUSED;
        };
        for (hUint i=0; i<rcd.vertexLayoutCount; ++i) {
            rc->vtxDeclDesc[i].Stream = 0;
            rc->vtxDeclDesc[i].Offset = rcd.vertexLayout_[i].offset_;
            rc->vtxDeclDesc[i].Type = vtxDeclTypeConv(rcd.vertexLayout_[i].type_, rcd.vertexLayout_[i].elementCount_, rcd.vertexLayout_[i].normalised_);
            rc->vtxDeclDesc[i].Method = D3DDECLMETHOD_DEFAULT;
            rc->vtxDeclDesc[i].Usage = (BYTE)rcd.vertexLayout_[i].semantic_;
            rc->vtxDeclDesc[i].UsageIndex = (BYTE)rcd.vertexLayout_[i].semIndex_;
        }
        rc->vtxDeclDesc[rcd.vertexLayoutCount] = D3DDECL_END();

        return rc;
    }
    void destroyRenderCall(hRenderCall* rc) {
        enqueueRenderResourceDelete([=](){
            delete rc;
        });
    }

    void* allocTempRenderMemory( hUint32 size ) {
        return nullptr;
    }

    hCmdList* createCmdList() {
        return new (rtmp_malloc(sizeof(hCmdList))) hCmdList();
    }

    void      linkCmdLists(hCmdList* before, hCmdList* after, hCmdList* i) {
        i->next = before->next;
        i->prev = after->prev;
        before->next = i;
        after->prev = i;
    }

    void      detachCmdLists(hCmdList* i) {
        i->next->prev = i->prev;
        i->prev->next = i->next;
        i->next = i;
        i->prev = i;
    }

    hCmdList* nextCmdList(hCmdList* i) {
        return i->next;
    }


    void clear(hCmdList* cl, hColour colour, hFloat depth) {
        auto* cmd = new (cl->allocCmdMem(Op::CustomCall, sizeof(hRndrOpCustomCall<std::function<hUint()>>))) hRndrOpCustomCall<std::function<hUint()>>( [=]() {
            hcAssert(isRenderThread());
            d3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
                D3DCOLOR_ARGB((hInt)(colour.a_*255.f+.5f), (hInt)(colour.r_*255.f+.5f), (hInt)(colour.g_*255.f+.5f), (hInt)(colour.b_*255.f+.5f)),
                depth, 0);
            return 0;
        });
    }

    void setViewport(hCmdList* cl, hUint x, hUint y, hUint width, hUint height, hFloat minz, hFloat maxz) {
        auto* cmd = new (cl->allocCmdMem(Op::CustomCall, sizeof(hRndrOpCustomCall<std::function<hUint()>>))) hRndrOpCustomCall<std::function<hUint()>>( [=]() {
            hcAssert(isRenderThread());
            D3DVIEWPORT9 vp;
            vp.X = x; vp.Y = y;
            vp.Width = width; vp.Height = height;
            vp.MinZ = minz; vp.MaxZ = maxz;
            d3dDevice->SetViewport(&vp);
            return 0;
        });
    }

    void scissorRect(hCmdList* cl, hUint left, hUint top, hUint right, hUint bottom) {
        auto* cmd = new (cl->allocCmdMem(Op::CustomCall, sizeof(hRndrOpCustomCall<std::function<hUint()>>))) hRndrOpCustomCall<std::function<hUint()>>( [=]() {
            hcAssert(isRenderThread());
            RECT r;
            r.left = left; r.top = top;
            r.right = right; r.bottom = bottom;
            d3dDevice->SetScissorRect(&r);
            return 0;
        });
    }

    void draw(hCmdList* cl, hRenderCall* rc, Primative t, hUint prims, hUint vtx_offset) {
        auto* cmd = new (cl->allocCmdMem(Op::CustomCall, sizeof(hRndrOpCustomCall<std::function<hUint()>>))) hRndrOpCustomCall<std::function<hUint()>>( [=]() {
            hcAssert(isRenderThread());
            // Bind any unbound objects
            if (!rc->isBound()) rc->bind(d3dDevice);
            //Update constants from uniform buffers
            for (hUint i=0, n=rc->paramUpdateCount; i<n; ++i) {
                if (rc->paramUpdates[i].type & hRenderCall::ParamUpdate::Vertex) {
                    hUint32* reg_add = ((rc->paramUpdates[i].type & hRenderCall::ParamUpdate::Float) ? (hUint32*)rc->vertex->getFloatRegisterBaseAddress() : (hUint32*)rc->vertex->getIntRegisterBaseAddress());
                    void* ub_add = rc->uniBuffers[rc->paramUpdates[i].ubIndex]->getBase()+rc->paramUpdates[i].ubOffset;
                    hMemCpy(reg_add+rc->paramUpdates[i].regIndex, ub_add, rc->paramUpdates[i].byteSize);
                }
                if (rc->paramUpdates[i].type & hRenderCall::ParamUpdate::Fragment) {
                    hUint32* reg_add = ((rc->paramUpdates[i].type & hRenderCall::ParamUpdate::Float) ? (hUint32*)rc->pixel->getFloatRegisterBaseAddress() : (hUint32*)rc->pixel->getIntRegisterBaseAddress());
                    void* ub_add = rc->uniBuffers[rc->paramUpdates[i].ubIndex]->getBase()+rc->paramUpdates[i].ubOffset;
                    hMemCpy(reg_add+rc->paramUpdates[i].regIndex, ub_add, rc->paramUpdates[i].byteSize);
                }
            }
            //Update d3d device state
            d3dDevice->SetVertexShader(rc->vertex->vertexShader.get());
            d3dDevice->SetPixelShader(rc->pixel->pixelShader.get());
            rc->vertex->uploadConstants(d3dDevice);
            rc->pixel->uploadConstants(d3dDevice);
            for (hUint i=0, n=rc->samplerCount; i<n; ++i) {
                auto samp_i = rc->samplerStates[i].index;
                for (const auto& ss : rc->samplerStates[i].samplerState.states) {
                    d3dDevice->SetSamplerState(samp_i, ss.state, ss.value);
                }
                d3dDevice->SetTexture(samp_i, rc->samplerStates[i].texture->texture.get());
            }
            for (const auto& rs : rc->states) {
                d3dDevice->SetRenderState(rs.state, rs.value);
            }
            d3dDevice->SetVertexDeclaration(rc->vtxDecl.get());
            d3dDevice->SetStreamSource(0, rc->vb->vertexBuffer.get(), 0, rc->vb->elementSize);
            D3DPRIMITIVETYPE type;
            if (t == Primative::Triangles) type = D3DPT_TRIANGLELIST;
            else if (t == Primative::TriangleStrip) type = D3DPT_TRIANGLESTRIP;
            if (rc->ib) {
                d3dDevice->SetIndices(rc->ib->indexBuffer.get());
                d3dDevice->DrawIndexedPrimitive(type, 0, 0, rc->vb->elementCount, vtx_offset, prims);
            } else {
                d3dDevice->DrawPrimitive(type, vtx_offset, prims);
            }
            return 0;
        });
    }

    void flushUnibufferMemoryRange(hCmdList* cl, hUniformBuffer* ub, hUint offset, hUint size) {
        auto* cmd = new (cl->allocCmdMem(Op::CustomCall, sizeof(hRndrOpCustomCall<std::function<hUint()>>))) hRndrOpCustomCall<std::function<hUint()>>([=]() {
            ub->baseOffset = offset;
            return 0;
        });
    }

    void flushVertexBufferMemoryRange(hCmdList* cl, hVertexBuffer* vb, hUint offset, hUint size) {
        hcAssert(offset + size <= vb->elementCount*vb->elementSize);
        auto* cmd = new (cl->allocCmdMem(Op::CustomCall, sizeof(hRndrOpCustomCall<std::function<hUint()>>))) hRndrOpCustomCall<std::function<hUint()>>( [=]() {
            hcAssert(isRenderThread());
            if (!vb->isBound()) vb->bind(d3dDevice);
            void* ptr;
            auto hr = vb->vertexBuffer->Lock(offset, size, &ptr, D3DLOCK_NOOVERWRITE);
            hcAssert(hr == S_OK);
            hMemCpy(ptr, vb->data.get()+offset, size);
            vb->vertexBuffer->Unlock();
            return 0;
        });
        
    }

    hRenderFence* fence(hCmdList* cl) {
        lfds_freelist_element* e;
        hRenderFence* f;
        lfds_freelist_use(fenceFreeList);
        lfds_freelist_pop(fenceFreeList, &e);
        hcAssertMsg(e, "Ran out of fences. This is fatal.");
        lfds_freelist_get_user_data_from_element(e, (void**)&f);
        f->element = e;

        auto* cmd = new (cl->allocCmdMem(Op::CustomCall, sizeof(hRndrOpCustomCall<std::function<hUint()>>))) hRndrOpCustomCall<std::function<hUint()>>( [=]() {
            f->post();
            return 0;
        });
        return f;
    }

    void wait(hRenderFence* fence) {
        fence->wait();
        hAtomic::LWMemoryBarrier();
        lfds_freelist_push(fenceFreeList, fence->element);
    }

    void flush(hCmdList* cl) {
        lfds_queue_use(cmdListQueue);
        while (lfds_queue_enqueue(cmdListQueue, cl) == 0) {
            Device::ThreadYield();
        }
    }

    void finish() {

    }

    void endReturn(hCmdList* cl) {
        cl->allocCmdMem(Op::Return, 0);
    }

    void call(hCmdList* cl, hCmdList* tocall) {
        auto* cmd = (hRndrOpCall*)cl->allocCmdMem(Op::Call, sizeof(hRndrOpCall));
        cmd->jumpTo=tocall;
        d3d9::endReturn(tocall);
    }

    void swapBuffers(hCmdList* cl) {
        auto* cmd = new (cl->allocCmdMem(Op::CustomCall, sizeof(hRndrOpCustomCall<std::function<hUint()>>))) hRndrOpCustomCall<std::function<hUint()>>( [=]() {
            d3dDevice->EndScene();
            d3dDevice->Present(nullptr, nullptr, 0, nullptr);
            return 0;
        });
    }

    void renderDoFrame();

    void submitFrame(hCmdList* cl) {
        auto writeindex = fenceIndex;
        // wait for prev frame to finish
        if (frameFences[fenceIndex]) {
            d3d9::wait(frameFences[fenceIndex]);
            frameFences[fenceIndex] = nullptr;
        }

        hCmdList* ncl=cl->prev;

        hcAssert(frameFences[fenceIndex] == nullptr);
        frameFences[fenceIndex] = d3d9::fence(ncl);
        fenceIndex = (fenceIndex+1)%FRAME_COUNT;

        ncl->allocCmdMem(Op::Return, 0);
        flush(ncl);
        rtmp_frameend();
        //!!JM - Multithread this call
        if (!multiThreadedRenderer) {
            renderDoFrame();
        }
    }


    void doRenderCmds(hByte* cmds) {
        hByte* cmdptr = cmds;
        hByte* pcstack = nullptr;
        hBool end_of_cmds = hFalse;
        hcAssert(isRenderThread());
        d3dDevice->BeginScene();
        while (!end_of_cmds) {
            Op opcode = *((hRenderer::Op*)cmdptr);
            hUint32 cmdsize = *((hUint32*)(cmdptr + 4));
            cmdptr += OpCodeSize;
            hByte* nextcmdptr = cmdptr + cmdsize;
            /*
            NoOp = 0,
            Jump,
            Call,
            Return,
            CustomCall,
            */
            switch (opcode) {
            default: hcAssertFailMsg("Unknown/Unhandled Render Op Code:%d", opcode); break;
            case Op::NoOp: break;
            case Op::Call: {
                hRndrOpCall* c = (hRndrOpCall*)cmdptr;
                hcAssertMsg(!pcstack, "Call stack is only 1-level deep");
                pcstack = nextcmdptr;
                nextcmdptr = c->jumpTo->cmds;
            } break;
            case Op::Return: {
                if (pcstack) {
                    nextcmdptr = pcstack;
                    pcstack = nullptr;
                }
                else {
                    end_of_cmds = true;
                    nextcmdptr = nullptr;
                }
            } break;
            case Op::CustomCall: {
                hRndrOpCustomCallBase* c = (hRndrOpCustomCallBase*)cmdptr;
                c->execute();
            } break;
            }
            cmdptr = nextcmdptr;
        }
        
        rtFrameIdx = (rtFrameIdx + 1) % FRAME_COUNT;
        // do pending deletes
        for (hRenderDestructBase* i = pendingDeletes[rtFrameIdx].next, *n; i != &pendingDeletes[rtFrameIdx]; i = n) {
            n = i->next;
            delete i;
        }
        pendingDeletes[rtFrameIdx].next = pendingDeletes[rtFrameIdx].prev = &pendingDeletes[rtFrameIdx];
    }

    void renderDoFrame() {
        hCmdList* cmds = nullptr, *fcmds = nullptr, *ncmds = nullptr;
        while (1) {
            while (hRenderDestructBase* pd = dequeueRenderResourceDelete()) {
                hRenderDestructBase::linkLists(&pendingDeletes[rtFrameIdx], pendingDeletes[rtFrameIdx].next, pd);
            }
            
            if (!cmds) {
                lfds_queue_use(cmdListQueue);
                lfds_queue_dequeue(cmdListQueue, (void**)&ncmds);
                if (ncmds) {
                    fcmds = cmds = ncmds;
                } else {
                    Device::ThreadYield();
                }
            }

            if (!cmds && !multiThreadedRenderer) {
                break;
            } else if (cmds) {
                doRenderCmds(cmds->cmds);

                cmds = cmds->next;
                if (cmds == fcmds) {
                    cmds = fcmds = nullptr;
                }
            }
        }
    }
    /*
        Methods to be called between beginCmdList & endCmdList
    */

    void rendererFrameSubmit(hCmdList* cmdlists, hUint count) {}
    hFloat getLastGPUTime() { return 0.f; }

    hUint getParameterTypeByteSize(ShaderParamType type) {
        switch(type) {
        case ShaderParamType::Unknown:  return -1;
        case ShaderParamType::Float:    return 4;
        case ShaderParamType::Float2:   return 8;
        case ShaderParamType::Float3:   return 12;
        case ShaderParamType::Float4:   return 16;
        case ShaderParamType::Float22:  return 16;
        case ShaderParamType::Float23:  return 24;
        case ShaderParamType::Float24:  return 32;
        case ShaderParamType::Float32:  return 16;
        case ShaderParamType::Float33:  return 36;
        case ShaderParamType::Float34:  return 48;
        case ShaderParamType::Float42:  return 32;
        case ShaderParamType::Float43:  return 48;
        case ShaderParamType::Float44:  return 64;
        default:        return 0;
    }
}
}
    HEART_C_EXPORT
    void HEART_API hrt_initialiseRenderFunc(hIConfigurationVariables* config_vars, hRendererInterfaceInitializer* out_funcs) {
                //hRenderer::getRatio = d3d9::getRatio;
        configVars = config_vars;

        out_funcs->create = d3d9::create;
        out_funcs->destroy = d3d9::destroy;
        out_funcs->isProfileSupported = d3d9::isProfileSupported;
        out_funcs->getActiveProfile = d3d9::getActiveProfile;
        out_funcs->getRenderStats = d3d9::getRenderStats;
        out_funcs->compileShaderStageFromSource = d3d9::compileShaderStageFromSource;
        out_funcs->destroyShader = d3d9::destroyShader;
        out_funcs->createTexture2D = d3d9::createTexture2D;
        out_funcs->destroyTexture2D = d3d9::destroyTexture2D;
        out_funcs->createIndexBuffer = d3d9::createIndexBuffer;
        out_funcs->getIndexBufferMappingPtr = d3d9::getIndexBufferMappingPtr;
        out_funcs->destroyIndexBuffer = d3d9::destroyIndexBuffer;
        out_funcs->createVertexBuffer = d3d9::createVertexBuffer;
        out_funcs->getVertexBufferMappingPtr = d3d9::getVertexBufferMappingPtr;
        out_funcs->destroyVertexBuffer = d3d9::destroyVertexBuffer;
        out_funcs->createUniformBuffer = d3d9::createUniformBuffer;
        out_funcs->getUniformBufferLayoutInfo = d3d9::getUniformBufferLayoutInfo;
        out_funcs->getUniformBufferMappingPtr = d3d9::getUniformBufferMappingPtr;
        out_funcs->destroyUniformBuffer = d3d9::destroyUniformBuffer;
        out_funcs->createRenderCall = d3d9::createRenderCall;
        out_funcs->destroyRenderCall = d3d9::destroyRenderCall;
        out_funcs->allocTempRenderMemory = d3d9::allocTempRenderMemory;
        out_funcs->createCmdList = d3d9::createCmdList;
        out_funcs->linkCmdLists = d3d9::linkCmdLists;
        out_funcs->detachCmdLists = d3d9::detachCmdLists;
        out_funcs->nextCmdList = d3d9::nextCmdList;
        out_funcs->clear = d3d9::clear;
        out_funcs->setViewport = d3d9::setViewport;
        out_funcs->scissorRect = d3d9::scissorRect;
        out_funcs->draw = d3d9::draw;
        out_funcs->flushUnibufferMemoryRange = d3d9::flushUnibufferMemoryRange;
        out_funcs->flushVertexBufferMemoryRange = d3d9::flushVertexBufferMemoryRange;
        out_funcs->fence = d3d9::fence;
        out_funcs->wait = d3d9::wait;
        out_funcs->flush = d3d9::flush;
        out_funcs->finish = d3d9::finish;
        out_funcs->call = d3d9::call;
        out_funcs->endReturn = d3d9::endReturn;
        out_funcs->swapBuffers = d3d9::swapBuffers;
        out_funcs->submitFrame = d3d9::submitFrame;
        out_funcs->rendererFrameSubmit = d3d9::rendererFrameSubmit;
        out_funcs->getLastGPUTime = d3d9::getLastGPUTime;
        out_funcs->isRenderThread = d3d9::isRenderThread;
        out_funcs->getParameterTypeByteSize = d3d9::getParameterTypeByteSize;
    }
}}

