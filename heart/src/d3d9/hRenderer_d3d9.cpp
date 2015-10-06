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
#include "opengl/GLCaps.h"
#include "opengl/GLTypes.h"
#include "opengl/hRendererOpCodes_gl.h"
#include "cryptoMurmurHash.h"
#include "lfds/lfds.h"
#include <d3d9.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <memory>
#include <unordered_map>
#include <functional>

namespace Heart {    
namespace hRenderer {
    static const hUint  FRAME_COUNT = 3;
    static const hUint  RMEM_COUNT = FRAME_COUNT+1;

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
                    auto data_end = constants[i].regStart*sizeof(float)*4+constants[i].bytes;
                    if (data_end > float_reg_size_bytes) float_reg_size_bytes = (hUint32)data_end;
                } else if (constants[i].type == 1) {
                    auto data_end = constants[i].regStart*sizeof(hInt)*4+constants[i].bytes;
                    if (data_end > int_reg_size_bytes) int_reg_size_bytes = (hUint32)data_end;
                }
            }
            floatRegCount = float_reg_size_bytes/(sizeof(float)*4);
            intRegCount = int_reg_size_bytes/(sizeof(hInt)*4);
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

        hBool isBound() { 
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
        std::unique_ptr<hChar> shaderBlob;
        const ReflectionHeader* header;
        const Constant* constants;
        const hChar* strings;
        const DWORD* programBlob;
        hUint32 floatRegStart, floatRegCount;
        hUint32 intRegStart, intRegCount;
        std::unique_ptr<hFloat> floatRegs;
        std::unique_ptr<hInt> intRegs;
        // No support for bools...yet
        IDirect3DVertexShader9Ptr vertexShader;
        IDirect3DPixelShader9Ptr pixelShader;
    };

    struct hTexture2D {
        hTexture2D(hUint32 w, hUint32 h, hMipDesc* in_mips, hUint32 mip_count, DWORD in_format) 
            : d3dFormat(in_format)
            , baseWidth(w)
            , baseHeight(h)
            , levels(mip_count) {
            for (hUint32 i=0; i < mip_count; ++i) {
                if (!in_mips[i].data) {
                    break;
                }
                mips.emplace_back(new hByte[in_mips[i].size]);
                hMemCpy(mips[i].get(), in_mips[i].data, in_mips[i].size);
            }
        }

        hBool isBound() {
            return !!texture.get();
        }

        hBool bind(IDirect3DDevice9* d3d_device) {
            return hFalse;
        }

        DWORD d3dFormat;
        hUint32 baseWidth;
        hUint32 baseHeight;
        hUint32 levels;
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

        hBool isBound() {
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

        hBool isBound() {
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
            , size(in_size) {
            layout.resize(in_count);
            hMemCpy(layout.data(), in_layout, sizeof(hUniformLayoutDesc)*in_count);
        }

        std::unique_ptr<hByte> data;
        std::vector<hUniformLayoutDesc> layout;
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
            DWORD state;
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
            DWORD state;
            DWORD value;
        };
        struct SamplerState {
            hUint index;
            const hTexture2D* texture;
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

        hShaderStage* vertex;
        hShaderStage* pixel;
        hUint stateCount;
        RenderState states[24];
        hUint samplerCount;
        SamplerState* samplerStates;
        hUint uniBufferCount;
        const hUniformBuffer** uniBuffers;
        hUint paramUpdateCount;
        ParamUpdate* paramUpdates;
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

    	multiThreadedRenderer = !!hConfigurationVariables::getCVarUint("renderer.gl.multithreaded", 1);
        renderScratchMemSize = hConfigurationVariables::getCVarUint("renderer.scratchmemsize", 1*1024*1024);
    	fenceCount = hConfigurationVariables::getCVarUint("renderer.fencecount", 256);
        auto destruction_queue_size = hConfigurationVariables::getCVarUint("renderer.destroyqueuesize", 256);
        auto cmd_queue_size = hConfigurationVariables::getCVarUint("renderer.cmdlistqueuesize", 256);
        renderCommandThreshold = hConfigurationVariables::getCVarUint("renderer.commandthreshold", 32);
        
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
        DWORD intfmt;
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
        return new hUniformBuffer(structSize, layout, layout_count);
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

    hRenderCall* createRenderCall(const hRenderCallDesc& rcd) {
        static const hUint sampler_size = sizeof(hSamplerState);
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
            + (uniform_buffer_count*sizeof(void*));
        auto* ptr = (hByte*)hMalloc(rnd_call_size);
        auto* rc = new (ptr) hRenderCall();
        rc->samplerStates = (hRenderCall::SamplerState*)(ptr+sizeof(hRenderCall));
        rc->samplerCount = sampler_count;
        rc->uniBuffers = (const hUniformBuffer**)(ptr+(sizeof(hRenderCall)+(sampler_count*sampler_size)));
        rc->uniBufferCount = uniform_buffer_count;
        rc->paramUpdates = (hRenderCall::ParamUpdate*)(ptr+(sizeof(hRenderCall)+(sampler_count*sampler_size)+(uniform_buffer_count*sizeof(void*))));
        rc->paramUpdateCount = int_param_update_count+float_param_update_count;
        rc->vertex=rcd.vertex_;
        rc->pixel=rcd.fragment_;
        rc->states[0]  = { (DWORD)D3DRS_ALPHABLENDENABLE, (DWORD)(rcd.blend_.blendEnable_ ? TRUE : FALSE) };
        rc->states[1]  = { (DWORD)D3DRS_SRCBLEND, blendOpConv(rcd.blend_.srcBlend_) };
        rc->states[2]  = { (DWORD)D3DRS_DESTBLEND, blendOpConv(rcd.blend_.destBlend_) };
        rc->states[3]  = { (DWORD)D3DRS_BLENDOP, blendFuncConv(rcd.blend_.blendOp_) };
        rc->states[4]  = { (DWORD)D3DRS_SEPARATEALPHABLENDENABLE, (DWORD)(separate_alpha_blend ? TRUE : FALSE) };
        rc->states[5]  = { (DWORD)D3DRS_SRCBLENDALPHA, blendOpConv(rcd.blend_.srcBlendAlpha_) };
        rc->states[6]  = { (DWORD)D3DRS_DESTBLENDALPHA, blendOpConv(rcd.blend_.destBlendAlpha_) };
        rc->states[7]  = { (DWORD)D3DRS_BLENDOPALPHA, blendFuncConv(rcd.blend_.blendOpAlpha_) };
        rc->states[8]  = { (DWORD)D3DRS_ZENABLE, (DWORD)(rcd.depthStencil_.depthEnable_ ? TRUE : FALSE) };
        rc->states[9]  = { (DWORD)D3DRS_ZWRITEENABLE, (DWORD)(rcd.depthStencil_.depthEnable_ ? TRUE : FALSE) };
        rc->states[10] = { (DWORD)D3DRS_ZFUNC, funcCompConv(rcd.depthStencil_.depthFunc_) };
        rc->states[11] = { (DWORD)D3DRS_STENCILENABLE, rcd.depthStencil_.stencilEnable_ };
        rc->states[12] = { (DWORD)D3DRS_STENCILFAIL, stencilOpConv(rcd.depthStencil_.stencilFailOp_) };
        rc->states[13] = { (DWORD)D3DRS_STENCILZFAIL, stencilOpConv(rcd.depthStencil_.stencilDepthFailOp_) };
        rc->states[14] = { (DWORD)D3DRS_STENCILPASS, stencilOpConv(rcd.depthStencil_.stencilPassOp_) };
        rc->states[15] = { (DWORD)D3DRS_STENCILFUNC, funcCompConv(rcd.depthStencil_.stencilFunc_) };
        rc->states[16] = { (DWORD)D3DRS_STENCILREF, rcd.depthStencil_.stencilRef_ };
        rc->states[17] = { (DWORD)D3DRS_STENCILMASK, rcd.depthStencil_.stencilReadMask_ };
        rc->states[18] = { (DWORD)D3DRS_STENCILWRITEMASK, rcd.depthStencil_.stencilWriteMask_ };
        rc->states[19] = { (DWORD)D3DRS_FILLMODE, fillModeConv(rcd.rasterizer_.fillMode_) };
        rc->states[20] = { (DWORD)D3DRS_CULLMODE, cullModeConv(rcd.rasterizer_.cullMode_) };
        rc->states[21] = { (DWORD)D3DRS_DEPTHBIAS, intBitsToDWORD(rcd.rasterizer_.depthBias_) };
        rc->states[22] = { (DWORD)D3DRS_SLOPESCALEDEPTHBIAS, floatBitsToDWORD(rcd.rasterizer_.slopeScaledDepthBias_) };
        rc->states[23] = { (DWORD)D3DRS_SCISSORTESTENABLE, (DWORD)(rcd.rasterizer_.scissorEnable_ ? TRUE : FALSE) };

        auto addSampler = [&](hRenderCall* in_rc, hUint i, hUint sampler_reg, const hRenderCallDesc::hSamplerStateDesc& ss, const hTexture2D* in_tex) {
            hcAssertMsg(in_tex, "No texture bound to sampler.");
            in_rc->samplerStates[i].index = sampler_reg;
            in_rc->samplerStates[i].texture = in_tex;
            in_rc->samplerStates[i].samplerState.states[0] = { (DWORD)D3DSAMP_ADDRESSU, (DWORD)textureAddConv(ss.addressU_)};
            in_rc->samplerStates[i].samplerState.states[1] = { (DWORD)D3DSAMP_ADDRESSV, (DWORD)textureAddConv(ss.addressV_)};
            in_rc->samplerStates[i].samplerState.states[2] = { (DWORD)D3DSAMP_ADDRESSW, (DWORD)textureAddConv(ss.addressW_)};
            in_rc->samplerStates[i].samplerState.states[3] = { (DWORD)D3DSAMP_BORDERCOLOR, (hUint32)ss.borderColour_};
            in_rc->samplerStates[i].samplerState.states[4] = { (DWORD)D3DSAMP_MAGFILTER, (DWORD)textureFilterConv(ss.filter_)};
            in_rc->samplerStates[i].samplerState.states[5] = { (DWORD)D3DSAMP_MINFILTER, (DWORD)textureFilterConv(ss.filter_)};
            in_rc->samplerStates[i].samplerState.states[6] = { (DWORD)D3DSAMP_MAXANISOTROPY, ss.maxAnisotropy_};
            in_rc->samplerStates[i].samplerState.states[7] = { (DWORD)D3DSAMP_MAXMIPLEVEL, floatBitsToDWORD(ss.maxLOD_)};
            in_rc->samplerStates[i].samplerState.states[8] = { (DWORD)D3DSAMP_MIPMAPLODBIAS, floatBitsToDWORD(ss.mipLODBias_)};
        };
        auto findMatchingSampler = [](const hRenderCallDesc& in_rcd, const hChar* name) -> const hRenderCallDesc::hSamplerStateDesc* {
            for (const auto& i : in_rcd.samplerStates_) {
                if (!i.name_.is_default() && hStrCmp(i.name_.c_str(), name) == 0) {
                    return &i.sampler_;
                }
            }
            return nullptr;
        };
        auto findMatchingTexture = [](const hRenderCallDesc& in_rcd, const hChar* name) -> const hTexture2D* {
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
            d3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
                D3DCOLOR_ARGB((hInt)(colour.a_*255.f+.5f), (hInt)(colour.r_*255.f+.5f), (hInt)(colour.g_*255.f+.5f), (hInt)(colour.b_*255.f+.5f)),
                depth, 0);
            return 0;
        });
    }

    void scissorRect(hCmdList* cl, hUint left, hUint top, hUint right, hUint bottom) {

    }

    void draw(hCmdList* cl, hRenderCall* rc, Primative t, hUint prims, hUint vtx_offset) {

    }

    void flushUnibufferMemoryRange(hCmdList* cl, hUniformBuffer* ub, hUint offset, hUint size) {

    }

    void flushVertexBufferMemoryRange(hCmdList* cl, hVertexBuffer* ub, hUint offset, hUint size) {

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

    void call(hCmdList* cl, hCmdList* tocall) {
        auto* cmd = (hRndrOpCall*)cl->allocCmdMem(Op::Call, sizeof(hRndrOpCall));
        cmd->jumpTo=tocall;
        endReturn(tocall);
    }

    void endReturn(hCmdList* cl) {
        cl->allocCmdMem(Op::Return, 0);
    }

    void swapBuffers(hCmdList* cl) {
        auto* cmd = new (cl->allocCmdMem(Op::CustomCall, sizeof(hRndrOpCustomCall<std::function<hUint()>>))) hRndrOpCustomCall<std::function<hUint()>>( [=]() {
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
    hBool isRenderThread() {
        return hFalse;
    }

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
    void initialiseRenderFunc() {
        hRenderer::create = d3d9::create;
        hRenderer::destroy = d3d9::destroy;
        //hRenderer::getRatio = d3d9::getRatio;
        hRenderer::isProfileSupported = d3d9::isProfileSupported;
        hRenderer::getActiveProfile = d3d9::getActiveProfile;
        hRenderer::getRenderStats = d3d9::getRenderStats;
        hRenderer::compileShaderStageFromSource = d3d9::compileShaderStageFromSource;
        hRenderer::destroyShader = d3d9::destroyShader;
        hRenderer::createTexture2D = d3d9::createTexture2D;
        hRenderer::destroyTexture2D = d3d9::destroyTexture2D;
        hRenderer::createIndexBuffer = d3d9::createIndexBuffer;
        hRenderer::getIndexBufferMappingPtr = d3d9::getIndexBufferMappingPtr;
        hRenderer::destroyIndexBuffer = d3d9::destroyIndexBuffer;
        hRenderer::createVertexBuffer = d3d9::createVertexBuffer;
        hRenderer::getVertexBufferMappingPtr = d3d9::getVertexBufferMappingPtr;
        hRenderer::destroyVertexBuffer = d3d9::destroyVertexBuffer;
        hRenderer::createUniformBuffer = d3d9::createUniformBuffer;
        hRenderer::getUniformBufferLayoutInfo = d3d9::getUniformBufferLayoutInfo;
        hRenderer::getUniformBufferMappingPtr = d3d9::getUniformBufferMappingPtr;
        hRenderer::destroyUniformBuffer = d3d9::destroyUniformBuffer;
        hRenderer::createRenderCall = d3d9::createRenderCall;
        hRenderer::destroyRenderCall = d3d9::destroyRenderCall;
        hRenderer::allocTempRenderMemory = d3d9::allocTempRenderMemory;
        hRenderer::createCmdList = d3d9::createCmdList;
        hRenderer::linkCmdLists = d3d9::linkCmdLists;
        hRenderer::detachCmdLists = d3d9::detachCmdLists;
        hRenderer::nextCmdList = d3d9::nextCmdList;
        hRenderer::clear = d3d9::clear;
        hRenderer::scissorRect = d3d9::scissorRect;
        hRenderer::draw = d3d9::draw;
        hRenderer::flushUnibufferMemoryRange = d3d9::flushUnibufferMemoryRange;
        hRenderer::flushVertexBufferMemoryRange = d3d9::flushVertexBufferMemoryRange;
        hRenderer::fence = d3d9::fence;
        hRenderer::wait = d3d9::wait;
        hRenderer::flush = d3d9::flush;
        hRenderer::finish = d3d9::finish;
        hRenderer::call = d3d9::call;
        hRenderer::endReturn = d3d9::endReturn;
        hRenderer::swapBuffers = d3d9::swapBuffers;
        hRenderer::submitFrame = d3d9::submitFrame;
        hRenderer::rendererFrameSubmit = d3d9::rendererFrameSubmit;
        hRenderer::getLastGPUTime = d3d9::getLastGPUTime;
        hRenderer::isRenderThread = d3d9::isRenderThread;
        hRenderer::getParameterTypeByteSize = d3d9::getParameterTypeByteSize;
    }
}}

