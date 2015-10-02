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
            programBlob = (void*)(shaderBlob.get()+header->totalLen);
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

        hShaderProfile profile;
        std::unique_ptr<hChar> shaderBlob;
        const ReflectionHeader* header;
        const Constant* constants;
        const hChar* strings;
        const void* programBlob;
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
            , data(new hByte[in_elementsize*in_elementcount]) {
            if (in_data) {
                hMemCpy(data.get(), in_data, in_elementsize*in_elementcount);
            }
        }

        hUint32 elementSize;
        hUint32 elementCount;
        std::unique_ptr<hByte> data;
        IDirect3DVertexBuffer9Ptr vertexBuffer;
    };

    struct hIndexBuffer {
        hIndexBuffer(hUint32 in_size, const hByte* in_data)
            : size(in_size)
            , data(new hByte[in_size]) {
            if (in_data) {
                hMemCpy(data.get(), in_data, in_size);
            }
        }

        hUint32 size;
        std::unique_ptr<hByte> data;
        IDirect3DIndexBuffer9Ptr indexBuffer;
    };

    struct hUniformBuffer {
        hUniformBuffer(hUint32 in_size)
            : data(new hByte[in_size])
            , size(in_size) {

        }

        std::unique_ptr<hByte> data;
        hUint32 size;
        
    };

    struct hSamplerState {
        struct StateValue {
            DWORD state;
            DWORD value;
        };
        hUint stateCount;
        StateValue* states;
    };

    struct hRenderCall {
        struct RenderState {
            DWORD state;
            DWORD value;
        };
        struct SamplerState {
            hUint index;
            hSamplerState* samplerState;
        };
        struct ParamUpdate {
            enum {
                Float, Int, Bool
            };
            hUint ubIndex;
            hUint ubOffset;
            hUint vec4Size;
        };

        hShaderStage* vertex;
        hShaderStage* pixel;
        hUint stateCount;
        RenderState* states;
        hUint samplerCount;
        SamplerState* samplerStates;
        hUint uniBufferCount;
        hUniformBuffer** uniBuffers;
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
        return new hUniformBuffer(structSize);
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
        static const hUint sampler_size = 1;
        hUint sampler_count = 0;
        hUint param_update_count = 0;

        auto* rc = new hRenderCall();
        //rcd.
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
    }
}}

