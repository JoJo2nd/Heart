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
#include <GL/glew.h>    
#include <GL/gl.h>
#include <SDL.h>
#include <memory>
#include <unordered_map>
#include <functional>

namespace Heart {    

class hSystem;

namespace hRenderer {
namespace RenderPrivate {

	static const hUint  FRAME_COUNT = 3;
    static const hUint  RMEM_COUNT = FRAME_COUNT+1;

	hBool				multiThreadedRenderer = false;
    SDL_Window*         window_;
    SDL_GLContext       context_;
    hThread             renderThread_;

	SDL_GLContext       mtContext_ = nullptr;
    hConditionVariable  rtComsSignal;
    hMutex              rtMutex;

    hSize_t             tlsContext_;
    hUint               rtFrameIdx;

	lfds_freelist_state* fenceFreeList;
	hSize_t				 fenceCount;
	hRenderFence*		 fences;

    lfds_queue_state*   cmdListQueue;
    lfds_queue_state*   destructionQueue;

    hMutex              memAccess;
    hUint               memIndex;
    hUint               renderScratchMemSize;
    hByte*              renderScratchMem[RMEM_COUNT];
    hSize_t             renderScratchAllocd[RMEM_COUNT];

    hUint               renderCommandThreshold;
	hUint				fenceIndex;
	hRenderFence*		frameFences[FRAME_COUNT];
    hRenderDestructBase	pendingDeletes[FRAME_COUNT];

    GLCaps Caps;
    struct {
        hVertexBuffer* (*impl_createVertexBuffer)(const void* data, hUint32 elementsize, hUint32 elementcount, hUint32 flags);
        void(*impl_flushVertexBuffer)(hVertexBuffer* vb, hUint offset, hUint size);
        void(*impl_destroyVertexBuffer)(hVertexBuffer*);
        hUniformBuffer* (*impl_createUniformBuffer)(const void* initdata, hUint size, hUint32 flags);
        void (*impl_flushUniformBuffer)(hUniformBuffer* ub);
        void(*impl_destroyUniformBuffer)(hUniformBuffer* ub);
        hSize_t(*impl_getSamplerObjectSize)();
        void(*impl_genSamplerObjectInplace)(const hRenderCallDesc::hSamplerStateDesc& desc, hGLSampler* out, hSize_t osize);
        void(*impl_destroySamplerObjectInplace)(hGLSampler* data);
        void(*impl_bindSamplerObject)(hInt index, hGLSampler* so);
    } ft;
}

using namespace RenderPrivate;

// !!JM TODO: Improve these (make lock-free?), they are placeholder
void* rtmp_malloc(hSize_t size, hUint alignment/*=16*/) {
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

void hglEnsureTLSContext() {
    SDL_GLContext ctx = (SDL_GLContext)TLS::getKeyValue(tlsContext_);
	if (!ctx && multiThreadedRenderer) {
		SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
		SDL_GL_MakeCurrent(window_, mtContext_);
        ctx = SDL_GL_CreateContext(window_);
        TLS::setKeyValue(tlsContext_, ctx);
        SDL_GL_MakeCurrent(window_, ctx);
		if (GL_ARB_debug_output) {
			glDebugMessageCallbackARB([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, hGLDebugCallback_t userParam) {
				hcPrintf("OpenGL Debug Message: %s", message);
			}, nullptr);
		}
    }
}

SDL_GLContext hglTLSMakeCurrent() {
    hglEnsureTLSContext();
    return (SDL_GLContext)TLS::getKeyValue(tlsContext_);
}

void hglReleaseTLSContext() {
    SDL_GLContext ctx = (SDL_GLContext)TLS::getKeyValue(tlsContext_);
	if (ctx) {
        SDL_GL_DeleteContext(ctx);
        TLS::setKeyValue(tlsContext_, nullptr);
    }
}

void hGLSyncFlush() {
	GLsync fenceId = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 ); 
	GLenum result = GL_TIMEOUT_EXPIRED; 
	while(result != GL_ALREADY_SIGNALED && result != GL_CONDITION_SATISFIED) { 
		//5 Second timeout but we ignore timeouts and wait until all OpenGL commands are processed! 
		result = glClientWaitSync(fenceId, GL_SYNC_FLUSH_COMMANDS_BIT, GLuint64(5000000000));
	}
}

static void doRenderCmds(hByte* cmds) {
    auto sizeof_sampler = ft.impl_getSamplerObjectSize();
    hByte* cmdptr = cmds;
    hByte* pcstack = nullptr;
    hBool end_of_cmds = hFalse;
    while (!end_of_cmds) {
        Op opcode = *((hRenderer::Op*)cmdptr);
        hUint32 cmdsize = *((hUint32*)(cmdptr + 4));
        cmdptr += OpCodeSize;
        hByte* nextcmdptr = cmdptr + cmdsize;
        switch (opcode) {
        default: hcAssertFailMsg("Unknown Render Op Code:%d", opcode); break;
        case Op::NoOp: break;
        case Op::Call: {
            hGLCall* c = (hGLCall*)cmdptr;
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
        case Op::Clear: {
            hGLErrorScope();
            hGLClear* c = (hGLClear*)cmdptr;
            glClearColor(c->colour.r_, c->colour.g_, c->colour.b_, c->colour.a_);
            glClear(GL_COLOR_BUFFER_BIT);
        } break;
        case Op::Fence: {
            hGLErrorScope();
            hGLFence* c = (hGLFence*)cmdptr;
            auto f = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            glFlush();
            c->fence->sync = f;
            hAtomic::LWMemoryBarrier();
        } break;
        case Op::UniBufferFlush: {
            auto* c = (hGLUniBufferFlush*)cmdptr;
            c->ub->mappedOffset_ = c->offset;
            c->ub->mappedSize_ = c->size;
            if (ft.impl_flushUniformBuffer) {
                ft.impl_flushUniformBuffer(c->ub);
            }
        } break;
        case Op::VtxBufferFlush: {
            auto* c = (hGLVertexBufferFlush*)cmdptr;
            // vtx flush should never be push if this is null
            hcAssert(ft.impl_flushVertexBuffer);
            ft.impl_flushVertexBuffer(c->vb, c->offset, c->size);
        } break;
        case Op::SetScissor: {
            auto* c = (hGLScissor*)cmdptr;
            glScissor(c->x, c->y, c->width, c->height);
        } break;
        case Op::Draw: {
            hGLErrorScope();
#define hGetArgs(x) (x*)args; args+=sizeof(x)
#define hGetArgsS(x,c,s) (x*)args; args+=((s)*(c))
#define hGetArgsN(x,c) hGetArgsS(x, c, sizeof(x))
            hGLDraw* c = (hGLDraw*)cmdptr;
            hRenderCall* rc = c->rc;
            hByte* args = rc->opCodes_.get();

            if (rc->header_.blend) {
                glEnable(GL_BLEND);
                glBlendColor(1.0, 1.0, 1.0, 1.0);
                auto* blend = hGetArgs(hGLBlend);
                if (rc->header_.seperateAlpha) {
                    auto* alpha = hGetArgs(hGLBlend);
                    glBlendFuncSeparate(blend->src, blend->dest, alpha->src, alpha->dest);
                    glBlendEquationSeparate(blend->func, alpha->func);
                }
                else {
                    glBlendFunc(blend->src, blend->dest);
                    glBlendEquation(blend->func);
                }
            }
            else {
                glDisable(GL_BLEND);
            }

            if (rc->header_.depth) {
                auto* depth = hGetArgs(hGLDepth);
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(depth->func);
                glDepthMask(depth->mask == ~0u);
            }
            else {
                glDisable(GL_DEPTH_TEST);
            }
            //depth bais
            if (rc->header_.depthBais) {
                auto* depthbais = hGetArgs(hGLDepthBais);
                //depthbais->depthBias_ = rcd.rasterizer_.depthBias_;
                //depthbais->depthBiasClamp_ = rcd.rasterizer_.depthBiasClamp_;
                //depthbais->slopeScaledDepthBias_ = rcd.rasterizer_.slopeScaledDepthBias_;
                //depthbais->depthClipEnable_ = rcd.rasterizer_.depthClipEnable_;
            }
            //stencil
            if (rc->header_.stencil) {
                auto* stencil = hGetArgs(hGLStencil);
                //stencil->readMask_ = rcd.depthStencil_.stencilReadMask_;
                //stencil->writeMask_ = rcd.depthStencil_.stencilWriteMask_;
                //stencil->ref_ = rcd.depthStencil_.stencilRef_;
                //stencil->failOp_ = stenciloptogl(rcd.depthStencil_.stencilFailOp_);
                //stencil->depthFailOp_ = stenciloptogl(rcd.depthStencil_.stencilDepthFailOp_);
                //stencil->passOp_ = stenciloptogl(rcd.depthStencil_.stencilPassOp_);
                //stencil->func_ = comparetogl(rcd.depthStencil_.stencilFunc_);
            }
            else {
                glDisable(GL_STENCIL_TEST);
            }
            glUseProgram(rc->program_);

            if (rc->header_.scissor) {
                glEnable(GL_SCISSOR_TEST);
            }
            else {
                glDisable(GL_SCISSOR_TEST);
            }

            //sampler states
            if (rc->header_.samplerCount_) {
                auto* samplers = hGetArgsS(hGLSampler, rc->header_.samplerCount_, sizeof_sampler);
                for (hUint8 i = 0, n = rc->header_.samplerCount_; i < n; ++i) {
                    auto* sampler = (hGLSampler*)(((hByte*)samplers) + (i*sizeof_sampler));
                    glUniform1i(sampler->index, sampler->index);
                    ft.impl_bindSamplerObject(sampler->index, sampler);
                }
            }

            //textures
            if (rc->header_.textureCount_) {
                auto* textures = hGetArgsN(hGLTexture2D, rc->header_.textureCount_);
                for (hUint8 i = 0, n = rc->header_.textureCount_; i < n; ++i) {
                    glActiveTexture(GL_TEXTURE0 + textures[i].index_);
                    glBindTexture(textures[i].tex_->target_, textures[i].tex_->name);
                }
            }

            // uniform buffers
            auto* ubs = (hGLUniformBuffer*)nullptr;
            if (rc->header_.uniBufferCount_) {
                ubs = hGetArgsN(hGLUniformBuffer, rc->header_.uniBufferCount_);
                for (hUint8 i = 0, n = rc->header_.uniBufferCount_; i < n; ++i) {
                    glBindBufferRange(GL_UNIFORM_BUFFER, ubs[i].index_, ubs[i].ub_->name, ubs[i].ub_->mappedOffset_, ubs[i].ub_->mappedSize_);
                    //glBindBuffer(ubs[i].index_, ubs[i].ub_->name);
                }
            }

            // Uniform parameters
            for (hUint i = 0, n = rc->paramUpdateCount; i < n; ++i) {
                auto* dataub = rc->uniformBuffers[rc->paramUpdates[i].uniformBufferIndex];
                auto* dataptr = (hUint8*)(dataub->persistantMapping) + (dataub->mappedOffset_ + rc->paramUpdates[i].uniformBufferOffset);
                switch (rc->paramUpdates[i].type) {
                case hParamCall::a1f: glUniform1fv(rc->paramUpdates[i].location, 1, (float*)dataptr); break;
                case hParamCall::a2f: glUniform2fv(rc->paramUpdates[i].location, 1, (float*)dataptr); break;
                case hParamCall::a3f: glUniform3fv(rc->paramUpdates[i].location, 1, (float*)dataptr); break;
                case hParamCall::a4f: glUniform4fv(rc->paramUpdates[i].location, 1, (float*)dataptr); break;
                case hParamCall::a1i: glUniform1iv(rc->paramUpdates[i].location, 1, (int*)dataptr); break;
                case hParamCall::a2i: glUniform2iv(rc->paramUpdates[i].location, 1, (int*)dataptr); break;
                case hParamCall::a3i: glUniform3iv(rc->paramUpdates[i].location, 1, (int*)dataptr); break;
                case hParamCall::a4i: glUniform4iv(rc->paramUpdates[i].location, 1, (int*)dataptr); break;
                case hParamCall::a1ui: glUniform1uiv(rc->paramUpdates[i].location, 1, (hUint*)dataptr); break;
                case hParamCall::a2ui: glUniform2uiv(rc->paramUpdates[i].location, 1, (hUint*)dataptr); break;
                case hParamCall::a3ui: glUniform3uiv(rc->paramUpdates[i].location, 1, (hUint*)dataptr); break;
                case hParamCall::a4ui: glUniform4uiv(rc->paramUpdates[i].location, 1, (hUint*)dataptr); break;
                case hParamCall::m2x2f: glUniformMatrix2fv(rc->paramUpdates[i].location, 1, hFalse, (float*)dataptr); break;
                case hParamCall::m2x3f: glUniformMatrix2fv(rc->paramUpdates[i].location, 1, hFalse, (float*)dataptr); break;
                case hParamCall::m2x4f: glUniformMatrix2fv(rc->paramUpdates[i].location, 1, hFalse, (float*)dataptr); break;
                case hParamCall::m3x2f: glUniformMatrix3fv(rc->paramUpdates[i].location, 1, hFalse, (float*)dataptr); break;
                case hParamCall::m3x3f: glUniformMatrix3fv(rc->paramUpdates[i].location, 1, hFalse, (float*)dataptr); break;
                case hParamCall::m3x4f: glUniformMatrix3fv(rc->paramUpdates[i].location, 1, hFalse, (float*)dataptr); break;
                case hParamCall::m4x2f: glUniformMatrix4fv(rc->paramUpdates[i].location, 1, hFalse, (float*)dataptr); break;
                case hParamCall::m4x3f: glUniformMatrix4fv(rc->paramUpdates[i].location, 1, hFalse, (float*)dataptr); break;
                case hParamCall::m4x4f: glUniformMatrix4fv(rc->paramUpdates[i].location, 1, hFalse, (float*)dataptr); break;
                default: hcAssertFailMsg("Unexpected enum value!");
                }
            }

            // index
            if (rc->header_.index) {
                auto* ib = hGetArgs(hIndexBuffer);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->name);
            }

            // vertex buffer
            if (!(rc->flags & hRenderCall::VAOBound)) {
                hGLErrorScope();
                auto* va = hGetArgsN(hGLVtxAttrib, rc->header_.vtxAttCount_);
                glGenVertexArrays(1, &rc->vao);
                glBindVertexArray(rc->vao);

                for (hUint8 i = 0, n = rc->header_.vtxAttCount_; i < n; ++i) {
                    hGLErrorScope();
                    glBindBuffer(GL_ARRAY_BUFFER, rc->vtxBuf_->name);
                    glEnableVertexAttribArray(va[i].index_);
                    glVertexAttribPointer(va[i].index_, va[i].size_, va[i].type_, va[i].normalise, va[i].stride_, va[i].pointer_);
                }

                rc->flags |= hRenderCall::VAOBound;
            }
            else {
                hGLErrorScope();
                glBindVertexArray(rc->vao);
            }

                {
                    hGLErrorScope();
                    glDrawArrays(c->primType, c->vtxOffset, c->count);
                }
                break;
#undef hGetArgs
#undef hGetArgsS
#undef hGetArgsN
        }
        case Op::Swap: {
            hGLErrorScope();
            SDL_GL_SwapWindow(window_);
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

static void renderDoFrame() {
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

hUint32 renderThreadMain(void* param) {
    context_ = hglTLSMakeCurrent();

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

    tlsContext_ = TLS::createKey([](void* ctx) {
        SDL_GL_DeleteContext(ctx);
    });

    rtFrameIdx = 0;

    window_ = system->getSDLWindow();

	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0);
	
	mtContext_ = SDL_GL_CreateContext(window_);
	if (multiThreadedRenderer == false) {
		TLS::setKeyValue(tlsContext_, mtContext_);
	}

	glewExperimental = GL_TRUE;
	GLenum result = glewInit();
	if (result != GLEW_OK) {
		hcPrintf("glewInit() error %d [%s]", result, glewGetErrorString(result));
	}

	if (GL_ARB_debug_output) {
		glDebugMessageCallbackARB([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, hGLDebugCallback_t userParam) {
			hcPrintf("OpenGL Debug Message: %s", message);
		}, nullptr);
	}

    //check for extensions
    initialiseOpenGLCaps(&Caps);
    hZeroMem(&ft, sizeof(ft));
    if (Caps.BufferStorage) {
        ft.impl_createVertexBuffer = Caps.BufferStorageProc.createVertexBuffer;
        ft.impl_destroyVertexBuffer = Caps.BufferStorageProc.destroyVertexBuffer;
        ft.impl_createUniformBuffer = Caps.BufferStorageProc.createUniformBuffer;
        ft.impl_destroyUniformBuffer = Caps.BufferStorageProc.destroyUniformBuffer;
    } else {
        ft.impl_createVertexBuffer = GLExt::Fallback::createVertexBuffer;
        ft.impl_flushVertexBuffer = GLExt::Fallback::flushVertexBuffer;
        ft.impl_destroyVertexBuffer = GLExt::Fallback::destroyVertexBuffer;
        ft.impl_createUniformBuffer = GLExt::Fallback::createUniformBuffer;
        ft.impl_flushUniformBuffer = GLExt::Fallback::flushUniformBuffer;
        ft.impl_destroyUniformBuffer = GLExt::Fallback::destroyUniformBuffer;
    }
    if (Caps.SamplerObjects) {
        ft.impl_getSamplerObjectSize = Caps.SamplerObjectsProc.getSamplerObjectSize;
        ft.impl_genSamplerObjectInplace = Caps.SamplerObjectsProc.genSamplerObjectInplace;
        ft.impl_destroySamplerObjectInplace = Caps.SamplerObjectsProc.destroySamplerObjectInplace;
        ft.impl_bindSamplerObject = Caps.SamplerObjectsProc.bindSamplerObject;
    } else {
        hcAssertFailMsg("TODO: Fallback for SamplerObjects");
    }

	if (multiThreadedRenderer == true) {
		SDL_GL_MakeCurrent(window_, nullptr);

		renderThread_.create("OpenGL Render Thread", hThread::PRIORITY_NORMAL, hFUNCTOR_BINDSTATIC(hThreadFunc, renderThreadMain), nullptr);

		rtMutex.Lock();
		rtComsSignal.wait(&rtMutex);
		rtMutex.Unlock();

		hglTLSMakeCurrent();
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void destroy() {
    hglReleaseTLSContext();
    context_=nullptr;
}

GLuint hglProfileToType(hShaderProfile profile) {
    switch (profile) {
    case hShaderProfile::ES2_vs: return GL_VERTEX_SHADER;
    case hShaderProfile::ES2_ps: return GL_FRAGMENT_SHADER;
    case hShaderProfile::WebGL_vs: return GL_VERTEX_SHADER;
    case hShaderProfile::WebGL_ps: return GL_FRAGMENT_SHADER;
    default: return GL_INVALID_VALUE;
    }
}

bool isProfileSupported(hShaderProfile profile) {
    return hglProfileToType(profile) != GL_INVALID_VALUE;
}

static GLenum hglToPrimType(Primative pt, hUint* count) {
	switch (pt) {
	case Primative::Triangles: (*count) *= 3; return GL_TRIANGLES;
	case Primative::TriangleStrip: (*count) += 2; return GL_TRIANGLE_STRIP;
	default: return GL_INVALID_VALUE;
	}
}

hShaderStage* createShaderStage(const hChar* shaderProg, hUint32 len, hShaderType type) {
    return nullptr;
}

hShaderStage* compileShaderStageFromSource(const hChar* shaderProg, hUint32 len, const hChar* entry, hShaderProfile profile) {
    hglEnsureTLSContext();
    hGLErrorScope();
    auto gls = glCreateShader(hglProfileToType(profile));
    if (!gls)
        return nullptr;
    auto gllen = (GLint)len;
    glShaderSource(gls, 1, &shaderProg, &gllen);
    glCompileShader(gls);
    // to check...but should we defer this until as late as possible i.e. RenderCall create time ???
    GLint status;
    glGetShaderiv(gls, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char slog[4096];
        glGetShaderInfoLog(gls, sizeof(slog), nullptr, slog);
        hcPrintf("Compile error: %s", slog);
    }

    hcAssert(status == GL_TRUE);
    
	hGLSyncFlush();
    auto* s = new hShaderStage();
    s->shaderObj_ = gls;
    return s;
}

void destroyShader(hShaderStage* shader) {
    //deletes are deferred, render thread will do it
    enqueueRenderResourceDelete([=]() {
        glDeleteShader(shader->shaderObj_);
        delete shader;
    });
}

hIndexBuffer* createIndexBuffer(const void* data, hUint32 nIndices, hUint32 flags) {
    hglEnsureTLSContext();
    hGLErrorScope();
    GLuint bname;
    glGenBuffers(1, &bname);
    if (!bname) {
        return nullptr;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bname);
    GLuint size = nIndices * (flags & (hUint32)hIndexBufferFlags::DwordIndices ? 4 : 2);
    glBufferData(GL_ARRAY_BUFFER, size, data, (flags & (hUint32)hIndexBufferFlags::DynamicBuffer) ?  GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	hGLSyncFlush();
    auto* ib = new hIndexBuffer();
    ib->name = bname;
    ib->indices = nIndices;
    ib->createFlags_ = flags;
    return ib;
}

void destroyIndexBuffer(hIndexBuffer* ib) {
    //deletes are deferred, render thread will do it
    enqueueRenderResourceDelete([=]() {
        glDeleteBuffers(1, &ib->name);
        delete ib;
    });
}

hVertexBuffer*  createVertexBuffer(const void* data, hUint32 elementsize, hUint32 elementcount, hUint32 flags) {
    return ft.impl_createVertexBuffer(data, elementsize, elementcount, flags);
}

void* getMappingPtr(hVertexBuffer* vb, hUint32* outsize) {
    hcAssert(vb);
    if (outsize) {
        *outsize = vb->alignedSize;
    }
    return vb->persistantMapping;
}

void  destroyVertexBuffer(hVertexBuffer* vb) {
    //deletes are deferred, render thread will do it
    enqueueRenderResourceDelete([=]() {
        ft.impl_destroyVertexBuffer(vb);
    });
}

hTexture2D*  createTexture2D(hUint32 levels, hMipDesc* initdata, hTextureFormat format, hUint32 flags) {
    hglEnsureTLSContext();
    hGLErrorScope();
    GLuint tname;
    GLuint fmt = GL_INVALID_VALUE;
    GLuint intfmt = GL_INVALID_VALUE;
    GLuint type = GL_INVALID_VALUE;
    hBool compressed = false;
    switch(format) {
    case hTextureFormat::R8_unorm:         intfmt=GL_R8;                                  fmt=GL_RED;  type=GL_UNSIGNED_BYTE; compressed = false; break; 
    case hTextureFormat::RGBA8_unorm:      intfmt=GL_RGBA8;                               fmt=GL_RGBA; type=GL_UNSIGNED_BYTE; compressed = false; break; 
    case hTextureFormat::RGBA8_sRGB_unorm: intfmt=GL_SRGB8_ALPHA8;                        fmt=GL_RGBA; type=GL_UNSIGNED_BYTE; compressed = false; break; 
    case hTextureFormat::BC1_unorm:        intfmt=GL_COMPRESSED_RGB_S3TC_DXT1_EXT;        fmt=GL_RGB;  type=GL_UNSIGNED_BYTE; compressed = true;  break;
    case hTextureFormat::BC2_unorm:        intfmt=GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;       fmt=GL_RGB;  type=GL_UNSIGNED_BYTE; compressed = true;  break;
    case hTextureFormat::BC3_unorm:        intfmt=GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;       fmt=GL_RGB;  type=GL_UNSIGNED_BYTE; compressed = true;  break;
    case hTextureFormat::BC4_unorm:        intfmt=GL_COMPRESSED_RED_RGTC1;                fmt=GL_RED;  type=GL_UNSIGNED_BYTE; compressed = true;  break;
    case hTextureFormat::BC5_unorm:        intfmt=GL_COMPRESSED_RG_RGTC2;                 fmt=GL_RG;   type=GL_UNSIGNED_BYTE; compressed = true;  break;
    case hTextureFormat::BC1_sRGB_unorm:   intfmt=GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;       fmt=GL_RGB;  type=GL_UNSIGNED_BYTE; compressed = true;  break;
    case hTextureFormat::BC2_sRGB_unorm:   intfmt=GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT; fmt=GL_RGB;  type=GL_UNSIGNED_BYTE; compressed = true;  break;
    case hTextureFormat::BC3_sRGB_unorm:   intfmt=GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT; fmt=GL_RGB;  type=GL_UNSIGNED_BYTE; compressed = true;  break;
    default: hcAssertFailMsg("Can't handle texture format"); return nullptr;
    }

    glActiveTexture(GL_TEXTURE0 + (RenderPrivate::Caps.MaxTextureUnits - 1));
    glGenTextures(1, &tname);
    glBindTexture(GL_TEXTURE_2D, tname);

    if (Caps.ImmutableTextureStorage) {
        Caps.ImmutableTextureStorageProc.create2D(levels, compressed, intfmt, fmt, type, initdata);
    } else {
        if (compressed) {
            for (auto i=0u; i<levels; ++i) {
                glCompressedTexImage2D(GL_TEXTURE_2D, i, intfmt, initdata[i].width, initdata[i].height, 0, initdata[i].size, initdata[i].data);
            }
        } else {
            for (auto i=0u; i<levels; ++i) {
                glTexImage2D(GL_TEXTURE_2D, i, intfmt, initdata[i].width, initdata[i].height, 0, fmt, type, initdata[i].data);
            }
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);
	hGLSyncFlush();
    auto* t = new hTexture2D();
    t->name = tname;
    t->target_ = GL_TEXTURE_2D;
    t->internalFormat_ = intfmt;
    t->format_ = fmt;
    return t;
}

void  destroyTexture2D(hTexture2D* t) {
    enqueueRenderResourceDelete([=]() {
        glDeleteTextures(1, &t->name);
        delete t;
    });
}

hUniformBuffer* createUniformBuffer(const void* initdata, const hUniformLayoutDesc* layout, hUint layout_count, hUint structSize, hUint bufferCount, hUint32 flags) {
	hUint infobufsize = 0;
    for (auto i=0u, n=layout_count; i<n; ++i) {
    	infobufsize += hStrLen(layout[i].fieldName)+1;
    }
    infobufsize += sizeof(hUniformLayoutDesc)*layout_count;
//     if (dynamic) {
//         structSize = hAlign(info.size, RenderPrivate::Caps.UniformBufferOffsetAlignment);
//     }
    auto* ub = ft.impl_createUniformBuffer(initdata, structSize*bufferCount, flags);
    ub->layoutDesc = (hUniformLayoutDesc*)hMalloc(infobufsize);
    ub->layoutDescCount = layout_count;
    hMemCpy(ub->layoutDesc, layout, sizeof(hUniformLayoutDesc)*layout_count);
    infobufsize = sizeof(hUniformLayoutDesc)*layout_count;
    auto* baseptr = (hByte*)ub->layoutDesc;
    for (auto i=0u, n=layout_count; i<n; ++i) {
    	ub->layoutDesc[i].fieldName = (char*)(baseptr + infobufsize);
        auto len=hStrLen(layout[i].fieldName)+1;
    	hStrCopy(ub->layoutDesc[i].fieldName, len, layout[i].fieldName);
    	infobufsize += len;
    }
    return ub;
}

void* getMappingPtr(hUniformBuffer* ub) {
	hcAssertMsg(ub && ub->persistantMapping, "Buffer is not dynamic!");
	return ub->persistantMapping;
}

void destroyUniformBuffer(hUniformBuffer* ub) {
    enqueueRenderResourceDelete([=]() {
        ft.impl_destroyUniformBuffer(ub);
    });
}

hRenderCall* createRenderCall(const hRenderCallDesc& rcd) {
    hglEnsureTLSContext();
    auto* rc = new hRenderCall();
    GLint p;

    rc->size_ = 0;
    rc->opCodes_ = nullptr;
    auto& header = rc->header_;
    hByte* currentOpCodes = nullptr;
#if HEART_DEBUG
    static const auto opguard = 16u;
    currentOpCodes = (hByte*)hRealloc(currentOpCodes, opguard);
    for (auto i=0u; i<opguard; ++i) currentOpCodes[i] = 0xBB;
#else
    static const auto opguard = 0u;
#endif

    rc->program_=glCreateProgram();
    glAttachShader(rc->program_, rcd.vertex_->shaderObj_);
    glAttachShader(rc->program_, rcd.fragment_->shaderObj_);
    glLinkProgram(rc->program_);

    glGetProgramiv(rc->program_, GL_LINK_STATUS, &p);
    hcAssert(p==GL_TRUE);

    auto ublimit = 0;
    auto ubtotal = 0;
    glGetProgramiv(rc->program_, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &ublimit);
    glGetProgramiv(rc->program_, GL_ACTIVE_UNIFORM_BLOCKS, &ubtotal);
    std::unique_ptr<GLchar*[]> ubary(new hChar*[ubtotal]);
    std::unique_ptr<hChar[]> ubnames(new hChar[ubtotal*ublimit]);
    //auto* ubhashes = (hUint32*)hAlloca(ubtotal*sizeof(hUint32));

    for (auto i=0,n=ubtotal; i<n; ++i) {
        auto olen = 0;
        ubary[i] = ubnames.get()+(i*ublimit);
        glGetActiveUniformBlockName(rc->program_, i, ublimit, &olen, ubary[i]);
        //cyMurmurHash3_x86_32(ubary[i], olen, hGetMurmurHashSeed(), ubhashes+i);
    }
    header.uniBufferCount_ = ubtotal;

    auto getunibufindex = [&](const hStringID& id) -> hUint {
        for (auto i=0, n=ubtotal; i<n; ++i) {
            if (hStrCmp(ubary[i], id.c_str())==0) return i;
        }
        return ~0u;
    };

    auto alimit = 0;
    auto atotal = 0;
    glGetProgramiv(rc->program_, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &alimit);
    glGetProgramiv(rc->program_, GL_ACTIVE_ATTRIBUTES, &atotal);
    std::unique_ptr<GLchar*[]> aary(new hChar*[atotal]);
    std::unique_ptr<hChar[]> anames(new hChar[atotal*alimit]);
    std::unique_ptr<GLenum[]> atypes(new GLenum[atotal]);
    std::unique_ptr<GLint[]> asizes(new GLint[atotal]);
    std::unique_ptr<hUint32[]> aloc(new hUint32[atotal]);

    for (auto i=0,n=atotal; i<n; ++i) {
        auto olen = 0;
        aary[i] = anames.get()+(i*alimit);
        glGetActiveAttrib(rc->program_, i, alimit, &olen, asizes.get()+i, atypes.get()+i, aary[i]);
		aloc[i] = glGetAttribLocation(rc->program_, aary[i]);
        //cyMurmurHash3_x86_32(aary[i], olen, hGetMurmurHashSeed(), ahashes+i);
    }
    header.vtxAttCount_ = atotal;

    auto getattribindex = [&](const hStringID& id) -> hUint {
        for (auto i=0, n=atotal; i<n; ++i) {
            if (hStrCmp(aary[i], id.c_str())==0) return aloc[i];
        }
        return ~0u;
    };

    auto ulimit = 0;
    auto utotal = 0;
    glGetProgramiv(rc->program_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &ulimit);
    glGetProgramiv(rc->program_, GL_ACTIVE_UNIFORMS, &utotal);
    std::unique_ptr <GLchar*[]> uary(new GLchar*[utotal]);
    std::unique_ptr<hChar[]>  unames(new hChar[utotal*ulimit]);
    std::unique_ptr<GLenum[]> utypes(new GLenum[utotal]);
    std::unique_ptr<GLint[]>  usizes(new GLint[utotal]);
    std::unique_ptr <hUint32[]> uloc(new hUint32[utotal]);
    //auto* uhashes = (hUint32*)hAlloca(utotal*sizeof(hUint32));
    auto requiresUniformParamUpdates = false;

    glUseProgram(rc->program_);
    for (auto i=0,n=utotal; i<n; ++i) {
        auto olen = 0;
        uary[i] = unames.get()+(i*ulimit);
        glGetActiveUniform(rc->program_, i, ulimit, &olen, usizes.get()+i, utypes.get()+i, uary[i]);
        uloc[i] = glGetUniformLocation(rc->program_, uary[i]);
        //cyMurmurHash3_x86_32(uary[i], olen, hGetMurmurHashSeed(), uhashes+i);
        // !!JM TODO: more types handled?
        if (utypes[i] == GL_SAMPLER_1D || utypes[i] == GL_SAMPLER_2D || utypes[i] == GL_SAMPLER_3D) {
            glUniform1i(uloc[i], uloc[i]);
            ++header.samplerCount_;
            ++header.textureCount_;
        } else {
            requiresUniformParamUpdates = true;
        }
    }

    auto getuniformindex = [&](const hStringID& id) -> hUint {
        for (auto i=0, n=utotal; i<n; ++i) {
            if (hStrCmp(uary[i], id.c_str())==0) return uloc[i];
        }
        return ~0u;
    };

    //build op codes required
    //header
    header.blend = rcd.blend_.blendEnable_;
    header.seperateAlpha = rcd.blend_.srcBlendAlpha_ != rcd.blend_.srcBlend_ ||
                           rcd.blend_.destBlendAlpha_ != rcd.blend_.destBlend_ ||
                           rcd.blend_.blendOpAlpha_ != rcd.blend_.blendOp_;
    header.depth = rcd.depthStencil_.depthEnable_;
    header.stencil = rcd.depthStencil_.stencilEnable_;
    header.index = rcd.indexBuffer_ != nullptr;
    header.writeMask = rcd.blend_.renderTargetWriteMask_ != ~0u;
    header.fill = rcd.rasterizer_.fillMode_ == proto::renderstate::Solid;
    header.cullCW = rcd.rasterizer_.cullMode_ == proto::renderstate::CullClockwise;
    header.cullCCW = rcd.rasterizer_.cullMode_ == proto::renderstate::CullCounterClockwise;
    header.scissor = rcd.rasterizer_.scissorEnable_ > 0;
    header.depthBais = rcd.rasterizer_.depthBias_ != 0;

    auto blendoptogl = [](proto::renderstate::BlendOp a) -> GLenum {
        switch(a) {
        case proto::renderstate::BlendZero             : return GL_ZERO;
        case proto::renderstate::BlendOne              : return GL_ONE;
        case proto::renderstate::BlendSrcColour        : return GL_SRC_COLOR;
        case proto::renderstate::BlendInverseSrcColour : return GL_ONE_MINUS_SRC_COLOR;
        case proto::renderstate::BlendDestColour       : return GL_DST_COLOR;
        case proto::renderstate::BlendInverseDestColour: return GL_ONE_MINUS_DST_COLOR;
        case proto::renderstate::BlendSrcAlpha         : return GL_SRC_ALPHA;
        case proto::renderstate::BlendInverseSrcAlpha  : return GL_ONE_MINUS_SRC_ALPHA;
        case proto::renderstate::BlendDestAlpha        : return GL_DST_ALPHA;
        case proto::renderstate::BlendInverseDestAlpha : return GL_ONE_MINUS_DST_ALPHA;
        }
        hcAssertFailMsg("unknown blend op value");
        return GL_INVALID_VALUE;
    };
    auto blendfunctogl = [](proto::renderstate::BlendFunction a) -> GLenum {
        switch(a) {
        case proto::renderstate::Add : return GL_FUNC_ADD;
        case proto::renderstate::Sub : return GL_FUNC_SUBTRACT;
        case proto::renderstate::Min : return GL_MIN;
        case proto::renderstate::Max : return GL_MAX;
        }
        hcAssertFailMsg("unknown blend function value");
        return GL_INVALID_VALUE;
    };
    // blend state
    if (header.blend) {
        auto ns = (hUint32)(rc->size_+sizeof(hGLBlend));
        currentOpCodes = (hByte*)hRealloc(currentOpCodes, ns+opguard);
#if HEART_DEBUG
        for (auto i = 0u; i < opguard; ++i) currentOpCodes[ns + i] = 0xBB;
#endif
        auto* glblend = (hGLBlend*)(currentOpCodes+rc->size_);
        glblend->func = blendfunctogl(rcd.blend_.blendOp_);
        glblend->src = blendoptogl(rcd.blend_.srcBlend_);
        glblend->dest = blendoptogl(rcd.blend_.destBlend_);
        rc->size_ = ns;
    }
#if HEART_DEBUG
    for (auto i = 0u; i < opguard; ++i) hcAssert(currentOpCodes[rc->size_+i] == 0xBB);
#endif
    // sepearate alpha blend state
    if (header.seperateAlpha) {
        auto ns = (hUint32)(rc->size_+sizeof(hGLBlend));
        currentOpCodes = (hByte*)hRealloc(currentOpCodes, ns+opguard);
#if HEART_DEBUG
        for (auto i = 0u; i < opguard; ++i) currentOpCodes[ns + i] = 0xBB;
#endif
        auto* glblend = (hGLBlend*)(currentOpCodes+rc->size_);
        glblend->func = blendfunctogl(rcd.blend_.blendOpAlpha_);
        glblend->src = blendoptogl(rcd.blend_.srcBlendAlpha_);
        glblend->dest = blendoptogl(rcd.blend_.destBlendAlpha_);
        rc->size_ = ns;
    }
#if HEART_DEBUG
    for (auto i = 0u; i < opguard; ++i) hcAssert(currentOpCodes[rc->size_ + i] == 0xBB);
#endif

    auto comparetogl = [](proto::renderstate::FunctionCompare a) -> GLenum {
        switch(a) {
        case proto::renderstate::CompareNever       : return GL_NEVER;
        case proto::renderstate::CompareLess        : return GL_LESS;
        case proto::renderstate::CompareEqual       : return GL_EQUAL;
        case proto::renderstate::CompareLessEqual   : return GL_LEQUAL;
        case proto::renderstate::CompareGreater     : return GL_GREATER;
        case proto::renderstate::CompareNotEqual    : return GL_NOTEQUAL;
        case proto::renderstate::CompareGreaterEqual: return GL_GEQUAL;
        case proto::renderstate::CompareAlways      : return GL_ALWAYS;
        }
        hcAssertFailMsg("unknown blend function value");
        return GL_INVALID_VALUE;
    };
    //depth
    if (header.depth) {
        auto ns = (hUint32)(rc->size_+sizeof(hGLDepth));
        currentOpCodes = (hByte*)hRealloc(currentOpCodes, ns+opguard);
#if HEART_DEBUG
        for (auto i = 0u; i < opguard; ++i) currentOpCodes[ns + i] = 0xBB;
#endif
        auto* gldepth = (hGLDepth*)(currentOpCodes+rc->size_);
        gldepth->func = comparetogl(rcd.depthStencil_.depthFunc_);
        gldepth->mask = rcd.depthStencil_.depthWriteMask_;
        rc->size_=ns;
    }
#if HEART_DEBUG
    for (auto i = 0u; i < opguard; ++i) hcAssert(currentOpCodes[rc->size_ + i] == 0xBB);
#endif
    //depth bais
    if (header.depthBais) {
        auto ns = (hUint32)(rc->size_+sizeof(hGLDepthBais));
        currentOpCodes = (hByte*)hRealloc(currentOpCodes, ns+opguard);
#if HEART_DEBUG
        for (auto i = 0u; i < opguard; ++i) currentOpCodes[ns + i] = 0xBB;
#endif
        auto* gldepthbais = (hGLDepthBais*)(currentOpCodes+rc->size_);
        gldepthbais->depthBias_ = rcd.rasterizer_.depthBias_;
        gldepthbais->depthBiasClamp_ = rcd.rasterizer_.depthBiasClamp_;
        gldepthbais->slopeScaledDepthBias_ = rcd.rasterizer_.slopeScaledDepthBias_;
        gldepthbais->depthClipEnable_ = rcd.rasterizer_.depthClipEnable_;
        rc->size_=ns;
    }
#if HEART_DEBUG
    for (auto i = 0u; i < opguard; ++i) hcAssert(currentOpCodes[rc->size_ + i] == 0xBB);
#endif

    auto stenciloptogl =[](proto::renderstate::StencilOp a) -> GLenum {
        switch(a) {
        case proto::renderstate::StencilKeep   : return GL_KEEP;
        case proto::renderstate::StencilZero   : return GL_ZERO;
        case proto::renderstate::StencilReplace: return GL_REPLACE;
        case proto::renderstate::StencilIncSat : return GL_INCR;
        case proto::renderstate::StencilDecSat : return GL_DECR;
        case proto::renderstate::StencilInvert : return GL_INVERT;
        case proto::renderstate::StencilIncr   : return GL_INCR_WRAP;
        case proto::renderstate::StencilDecr   : return GL_DECR_WRAP;
        }
        hcAssertFailMsg("unknown StencilOp value");
        return GL_INVALID_VALUE;
    };
    //stencil
    if (header.stencil) {
        auto ns = (hUint32)(rc->size_+sizeof(hGLStencil));
        currentOpCodes = (hByte*)hRealloc(currentOpCodes, ns+opguard);
#if HEART_DEBUG
        for (auto i = 0u; i < opguard; ++i) currentOpCodes[ns + i] = 0xBB;
#endif
        auto* glstencil = (hGLStencil*)(currentOpCodes+rc->size_);
        glstencil->readMask_ = rcd.depthStencil_.stencilReadMask_;
        glstencil->writeMask_ = rcd.depthStencil_.stencilWriteMask_;
        glstencil->ref_ = rcd.depthStencil_.stencilRef_;
        glstencil->failOp_ = stenciloptogl(rcd.depthStencil_.stencilFailOp_);
        glstencil->depthFailOp_ = stenciloptogl(rcd.depthStencil_.stencilDepthFailOp_);
        glstencil->passOp_ = stenciloptogl(rcd.depthStencil_.stencilPassOp_);
        glstencil->func_ = comparetogl(rcd.depthStencil_.stencilFunc_);
        rc->size_=ns;
    }
#if HEART_DEBUG
    for (auto i = 0u; i < opguard; ++i) hcAssert(currentOpCodes[rc->size_ + i] == 0xBB);
#endif
    
    // sampler states
    if (header.samplerCount_) {
        auto sizeof_sampler = ft.impl_getSamplerObjectSize();
        auto ns = (hUint32)(rc->size_+(sizeof_sampler*header.samplerCount_));
        currentOpCodes = (hByte*)hRealloc(currentOpCodes, ns+opguard);
#if HEART_DEBUG
        for (auto i = 0u; i < opguard; ++i) currentOpCodes[ns + i] = 0xBB;
#endif
        rc->samplers_ = (hGLSampler*)(currentOpCodes+rc->size_);
        auto* wsamplers = rc->samplers_;
        for (auto i=0u, n=hRenderCallDesc::samplerStateMax_; i<n && !rcd.samplerStates_[i].name_.is_default(); ++i) {
            auto si = getuniformindex(rcd.samplerStates_[i].name_);
            if (si != ~0u) {
                wsamplers->index = si;
                ft.impl_genSamplerObjectInplace(rcd.samplerStates_[i].sampler_, wsamplers, sizeof_sampler);
                wsamplers = (hGLSampler*)(((hByte*)wsamplers)+sizeof_sampler);
            }
        }
        rc->size_=ns;    
    }
#if HEART_DEBUG
    for (auto i = 0u; i < opguard; ++i) hcAssert(currentOpCodes[rc->size_ + i] == 0xBB);
#endif

    //textures
    if (header.textureCount_) {
        auto ns = (hUint32)(rc->size_+(sizeof(hGLTexture2D)*header.textureCount_));
        currentOpCodes = (hByte*)hRealloc(currentOpCodes, ns+opguard);
#if HEART_DEBUG
        for (auto i = 0u; i < opguard; ++i) currentOpCodes[ns + i] = 0xBB;
#endif
        auto* textures = (hGLTexture2D*)(currentOpCodes+rc->size_);
        auto* wtextures = textures;
        for (auto i=0u, n=hRenderCallDesc::textureSlotMax_; i<n && !rcd.textureSlots_[i].name_.is_default(); ++i) {
            auto si = getuniformindex(rcd.textureSlots_[i].name_);
            if (si != ~0u) {
                // only support 2D textures atm, need to support more
                hcAssert(rcd.textureSlots_[i].texType_ == 2);
                wtextures->index_ = si;
                wtextures->tex_ = rcd.textureSlots_[i].t2D_; 
                ++wtextures;
            }
        }
        rc->size_=ns;    
    }
#if HEART_DEBUG
    for (auto i = 0u; i < opguard; ++i) hcAssert(currentOpCodes[rc->size_ + i] == 0xBB);
#endif

    // uniform buffers
    if (header.uniBufferCount_) {
        auto ns = (hUint32)(rc->size_+(sizeof(hGLUniformBuffer)*header.uniBufferCount_));
        currentOpCodes = (hByte*)hRealloc(currentOpCodes, ns+opguard);
#if HEART_DEBUG
        for (auto i = 0u; i < opguard; ++i) currentOpCodes[ns + i] = 0xBB;
#endif
        auto* buffers = (hGLUniformBuffer*)(currentOpCodes+rc->size_);
        auto* wbuffers = buffers;
        for (auto i=0u, n=hRenderCallDesc::uniformBufferMax_; i<n && !rcd.uniformBuffers_[i].name_.is_default(); ++i) {
            auto si = getunibufindex(rcd.uniformBuffers_[i].name_);
            if (si != ~0u) {
                wbuffers->index_ = si; 
                wbuffers->ub_ = rcd.uniformBuffers_[i].ub_; 
                ++wbuffers;
            }
        }
        rc->size_=ns;    
    }
#if HEART_DEBUG
    for (auto i = 0u; i < opguard; ++i) hcAssert(currentOpCodes[rc->size_ + i] == 0xBB);
#endif

    if (requiresUniformParamUpdates) {
        // walk the list of uniforms and find the parameters we need to copy across.
		auto maxubindex = 0u;
        std::vector<hUniformParamUpdate> updateparams;
        updateparams.reserve(utotal);
        for (hUint i=0, n=utotal; i<n; ++i) {
            if (utypes[i] == GL_SAMPLER_1D || utypes[i] == GL_SAMPLER_2D || utypes[i] == GL_SAMPLER_3D) {
                // Handled in other ways.
                continue;
            }
            hUniformParamUpdate param_update;
            hUint index, offset, size;
			ShaderParamType type;
            auto found = rcd.findNamedParameter(uary[i], &index, &offset, &size, &type);
            hcAssertMsg(found, "Parameter '%s' is not matched to an input. This will cause rendering issues!", uary[i]);
            if (found){
                hcAssertMsg((index & 0xFF) == index && (offset & 0xFFFF) == offset && (size & 0xFF) == size, "Uniform parameter is out of range");
				maxubindex = index > maxubindex ? index : maxubindex;
                param_update.uniformBufferIndex = (index & 0xFF);
                param_update.uniformBufferOffset = (offset & 0xFFFF);
                param_update.paramSize = (size & 0xFF);
				param_update.location = glGetUniformLocation(rc->program_, uary[i]);
                switch (type) {
                case ShaderParamType::Float     : param_update.type = hParamCall::a1f;   break;
                case ShaderParamType::Float2    : param_update.type = hParamCall::a2f;   break;
                case ShaderParamType::Float3    : param_update.type = hParamCall::a3f;   break;
                case ShaderParamType::Float4    : param_update.type = hParamCall::a4f;   break;
                case ShaderParamType::Float22   : param_update.type = hParamCall::m2x2f; break;
                case ShaderParamType::Float23   : param_update.type = hParamCall::m2x3f; break;
                case ShaderParamType::Float24   : param_update.type = hParamCall::m2x4f; break;
                case ShaderParamType::Float32   : param_update.type = hParamCall::m4x2f; break;
                case ShaderParamType::Float33   : param_update.type = hParamCall::m3x3f; break;
                case ShaderParamType::Float34   : param_update.type = hParamCall::m3x4f; break;
                case ShaderParamType::Float42   : param_update.type = hParamCall::m4x2f; break;
                case ShaderParamType::Float43   : param_update.type = hParamCall::m4x3f; break;
                case ShaderParamType::Float44   : param_update.type = hParamCall::m4x4f; break;
                default                         : param_update.type = hParamCall::none;  break;
                }
                updateparams.push_back(param_update);
            }
        }
		rc->uniformBuffers.resize(++maxubindex, nullptr);
		for (auto i=0u, n=hRenderCallDesc::uniformBufferMax_; i<maxubindex; ++i) {
			if (rcd.uniformBuffers_[i].name_.is_default()) {
				continue;
			}
			rc->uniformBuffers[i] = rcd.uniformBuffers_[i].ub_;
		}
        rc->paramUpdateCount = (hUint)updateparams.size();
        hcAssertMsg(updateparams.size() == rc->paramUpdateCount, "too many paramers!");
        rc->paramUpdates.reset(new hUniformParamUpdate[rc->paramUpdateCount]);
        hMemCpy(rc->paramUpdates.get(), updateparams.data(), sizeof(hUniformParamUpdate)*rc->paramUpdateCount);
    }

    // index
    if (header.index) {
        auto ns = (hUint32)(rc->size_+(sizeof(hIndexBuffer*)));
        currentOpCodes = (hByte*)hRealloc(currentOpCodes, ns+opguard);
#if HEART_DEBUG
        for (auto i = 0u; i < opguard; ++i) currentOpCodes[ns + i] = 0xBB;
#endif
        auto** ib = (hIndexBuffer**)(currentOpCodes+rc->size_);
        *ib = rcd.indexBuffer_;
        rc->size_=ns;
    }
#if HEART_DEBUG
    for (auto i = 0u; i < opguard; ++i) hcAssert(currentOpCodes[rc->size_ + i] == 0xBB);
#endif

    // vertex buffer
    rc->vtxBuf_ = rcd.vertexBuffer_;

    // vertex attributes
    {
        auto inputtypetogl = [](Heart::hRenderer::hVertexInputType a) -> GLenum {
            switch(a) {
            case hVertexInputType::Byte        : return GL_BYTE;
            case hVertexInputType::UByte       : return GL_UNSIGNED_BYTE;
            case hVertexInputType::Short       : return GL_SHORT;
            case hVertexInputType::UShort      : return GL_UNSIGNED_SHORT;
            case hVertexInputType::Int         : return GL_INT;
            case hVertexInputType::UInt        : return GL_UNSIGNED_INT;
            case hVertexInputType::HalfFloat   : return GL_HALF_FLOAT;
            case hVertexInputType::Float       : return GL_FLOAT;
            case hVertexInputType::Double      : return GL_DOUBLE;
            }
            hcAssertFailMsg("unknown StencilOp value");
            return GL_INVALID_VALUE;
        };

        auto ns = (hUint32)(rc->size_+(sizeof(hGLVtxAttrib)*header.vtxAttCount_));
        currentOpCodes = (hByte*)hRealloc(currentOpCodes, ns+opguard);
#if HEART_DEBUG
        for (auto i = 0u; i < opguard; ++i) currentOpCodes[ns + i] = 0xBB;
#endif
        auto* attribs = (hGLVtxAttrib*)(currentOpCodes+rc->size_);
        for (auto i=0u, n=hRenderCallDesc::vertexLayoutMax_; i<n && !rcd.vertexLayout_[i].bindPoint_.is_default(); ++i) {
            auto bindpoint = getattribindex(rcd.vertexLayout_[i].bindPoint_);
            if (bindpoint != ~0u) {
				hcAssertMsg(bindpoint < header.vtxAttCount_, "Vertex input index is too high");
                attribs[bindpoint].index_=bindpoint;
                attribs[bindpoint].size_=rcd.vertexLayout_[i].elementCount_;
                attribs[bindpoint].type_=inputtypetogl(rcd.vertexLayout_[i].type_);
                attribs[bindpoint].pointer_=(void*)rcd.vertexLayout_[i].offset_;
                attribs[bindpoint].stride_=rcd.vertexLayout_[i].stride_;
                attribs[bindpoint].normalise=rcd.vertexLayout_[i].normalised_;
            }
        }
        rc->size_=ns;
#if HEART_DEBUG
        for (auto i = 0u; i < opguard; ++i) hcAssert(currentOpCodes[rc->size_ + i] == 0xBB);
#endif
    }

    rc->opCodes_ = std::unique_ptr<hByte, std::function<void(hByte*)>>(currentOpCodes, [](hByte* p){hFree(p);});
	hGLSyncFlush();
    return rc;
}

void destroyRenderCall(hRenderCall* rc) {
    enqueueRenderResourceDelete([=]{
        // TODO: Can't this just be a destructor on the object??
        glDeleteProgram(rc->program_);
        auto sizeof_sampler = ft.impl_getSamplerObjectSize();
        for (hUint i=0,n=rc->header_.samplerCount_; i<n; ++i) {
            ft.impl_destroySamplerObjectInplace((hGLSampler*)(((hByte*)rc->samplers_)+(sizeof_sampler*i)));
        }
        delete rc;
    });
}

struct hProgramReflectionInfo {
    typedef std::unordered_map<hString, hShaderParamInfo> hParamHash;

    hProgramReflectionInfo() 
        : prog(0) {}

    GLuint                          prog;
    std::unique_ptr<hChar[]>        ubNames;
    std::vector<hUniformBlockInfo>  uniformBlocks;
    hParamHash                      paramTable;
};

static ShaderParamType mapGLParamType(GLenum t) {
    switch(t) {
    case GL_FLOAT: return ShaderParamType::Float;
    case GL_FLOAT_VEC2: return ShaderParamType::Float2;
    case GL_FLOAT_VEC3: return ShaderParamType::Float3;
    case GL_FLOAT_VEC4: return ShaderParamType::Float4;
    case GL_INT: return ShaderParamType::Unknown;
    case GL_INT_VEC2: return ShaderParamType::Unknown;
    case GL_INT_VEC3: return ShaderParamType::Unknown;
    case GL_INT_VEC4: return ShaderParamType::Unknown;
    case GL_UNSIGNED_INT: return ShaderParamType::Unknown;
    case GL_UNSIGNED_INT_VEC2: return ShaderParamType::Unknown;
    case GL_UNSIGNED_INT_VEC3: return ShaderParamType::Unknown;
    case GL_UNSIGNED_INT_VEC4: return ShaderParamType::Unknown;
    case GL_BOOL: return ShaderParamType::Unknown;
    case GL_BOOL_VEC2: return ShaderParamType::Unknown;
    case GL_BOOL_VEC3: return ShaderParamType::Unknown;
    case GL_BOOL_VEC4: return ShaderParamType::Unknown;
    case GL_FLOAT_MAT2: return ShaderParamType::Float22;
    case GL_FLOAT_MAT3: return ShaderParamType::Float33;
    case GL_FLOAT_MAT4: return ShaderParamType::Float44;
    case GL_FLOAT_MAT2x3: return ShaderParamType::Float23;
    case GL_FLOAT_MAT2x4: return ShaderParamType::Float24;
    case GL_FLOAT_MAT3x2: return ShaderParamType::Float32;
    case GL_FLOAT_MAT3x4: return ShaderParamType::Float34;
    case GL_FLOAT_MAT4x2: return ShaderParamType::Float42;
    case GL_FLOAT_MAT4x3: return ShaderParamType::Float43;
    case GL_SAMPLER_1D: return ShaderParamType::Sampler1D;
    case GL_SAMPLER_2D: return ShaderParamType::Sampler2D;
    case GL_SAMPLER_3D: return ShaderParamType::Sampler3D;
    case GL_SAMPLER_CUBE: return ShaderParamType::Unknown;
    case GL_SAMPLER_1D_SHADOW: return ShaderParamType::Unknown;
    case GL_SAMPLER_2D_SHADOW: return ShaderParamType::Unknown;
    case GL_SAMPLER_1D_ARRAY: return ShaderParamType::Unknown;
    case GL_SAMPLER_2D_ARRAY: return ShaderParamType::Unknown;
    case GL_SAMPLER_1D_ARRAY_SHADOW: return ShaderParamType::Unknown;
    case GL_SAMPLER_2D_ARRAY_SHADOW: return ShaderParamType::Unknown;
    case GL_SAMPLER_2D_MULTISAMPLE: return ShaderParamType::Unknown;
    case GL_SAMPLER_2D_MULTISAMPLE_ARRAY: return ShaderParamType::Unknown;
    case GL_SAMPLER_CUBE_SHADOW: return ShaderParamType::Unknown;
    case GL_SAMPLER_BUFFER: return ShaderParamType::Unknown;
    case GL_SAMPLER_2D_RECT: return ShaderParamType::Unknown;
    case GL_SAMPLER_2D_RECT_SHADOW: return ShaderParamType::Unknown;
    case GL_INT_SAMPLER_1D: return ShaderParamType::Unknown;
    case GL_INT_SAMPLER_2D: return ShaderParamType::Unknown;
    case GL_INT_SAMPLER_3D: return ShaderParamType::Unknown;
    case GL_INT_SAMPLER_CUBE: return ShaderParamType::SamplerCube;
    case GL_INT_SAMPLER_1D_ARRAY: return ShaderParamType::Unknown;
    case GL_INT_SAMPLER_2D_ARRAY: return ShaderParamType::Unknown;
    case GL_INT_SAMPLER_2D_MULTISAMPLE: return ShaderParamType::Unknown;
    case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return ShaderParamType::Unknown;
    case GL_INT_SAMPLER_BUFFER: return ShaderParamType::Unknown;
    case GL_INT_SAMPLER_2D_RECT: return ShaderParamType::Unknown;
    case GL_UNSIGNED_INT_SAMPLER_1D: return ShaderParamType::Unknown;
    case GL_UNSIGNED_INT_SAMPLER_2D: return ShaderParamType::Unknown;
    case GL_UNSIGNED_INT_SAMPLER_3D: return ShaderParamType::Unknown;
    case GL_UNSIGNED_INT_SAMPLER_CUBE: return ShaderParamType::Unknown;
    case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY: return ShaderParamType::Unknown;
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: return ShaderParamType::Unknown;
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE: return ShaderParamType::Unknown;
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return ShaderParamType::Unknown;
    case GL_UNSIGNED_INT_SAMPLER_BUFFER: return ShaderParamType::Unknown;
    case GL_UNSIGNED_INT_SAMPLER_2D_RECT: return ShaderParamType::Unknown;
    }
    return ShaderParamType::Unknown;
};

hProgramReflectionInfo* createProgramReflectionInfo(hShaderStage* vertex, hShaderStage* pixel, hShaderStage* geom, hShaderStage* hull, hShaderStage* domain) {
    hglEnsureTLSContext();
    hcAssertMsg(vertex && pixel, "vertex and pixel shaders are required.");
    hProgramReflectionInfo* refinfo=nullptr;
    auto p=glCreateProgram();
    auto status=0;
    glAttachShader(p, vertex->shaderObj_);
    glAttachShader(p, pixel->shaderObj_);
    if (geom)
        glAttachShader(p, geom->shaderObj_);
    if (hull)
        glAttachShader(p, hull->shaderObj_);
    if (domain)
        glAttachShader(p, domain->shaderObj_);
    glLinkProgram(p);
    glGetProgramiv(p, GL_LINK_STATUS, &status);
    if (status!=GL_TRUE) {
        glDeleteShader(p);
        delete refinfo;
        return nullptr;
    }

    refinfo = new hProgramReflectionInfo();
    auto namelimit = 0;
    auto unicount = 0;
    auto uniblocknamelimit = 0;
    auto uniblockcount = 0;
    glGetProgramiv(p, GL_ACTIVE_UNIFORM_MAX_LENGTH, &namelimit);
    glGetProgramiv(p, GL_ACTIVE_UNIFORMS, &unicount);
    glGetProgramiv(p, GL_ACTIVE_UNIFORM_BLOCKS, &uniblockcount);
    glGetProgramiv(p, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &uniblocknamelimit);
    refinfo->ubNames.reset(new hChar[uniblockcount*uniblocknamelimit]);
    hChar* uniname = (hChar*)hAlloca(namelimit);
    hUint32* hashes = (hUint32*)hAlloca(unicount*sizeof(hUint32));
    for (auto i=0,n=unicount; i<n; ++i) {
        GLsizei olen = 0;
        GLenum type = 0;
        GLint size = 0;
        glGetActiveUniform(p, i, namelimit, &olen, &size, &type, uniname);
        auto i2 = (GLuint)i;
        auto blockindex = 0;
        auto blockoffset = 0;
        glGetActiveUniformsiv(p, 1, &i2, GL_UNIFORM_BLOCK_INDEX, &blockindex);
        glGetActiveUniformsiv(p, 1, &i2, GL_UNIFORM_OFFSET, &blockoffset);

        hShaderParamInfo info = { (hUint)blockindex, (hUint)blockoffset, mapGLParamType(type), (hUint)size };
        refinfo->paramTable[uniname] = info;
    }

    refinfo->uniformBlocks.reserve(uniblockcount);
    for (auto i=0,n=uniblockcount; i<n; ++i) {
        hUniformBlockInfo info;
        info.index = i;
        info.name = refinfo->ubNames.get()+(i*uniblocknamelimit);
        glGetActiveUniformBlockName(p, i, uniblocknamelimit, nullptr, refinfo->ubNames.get()+(i*uniblocknamelimit));
        glGetActiveUniformBlockiv(p, i, GL_UNIFORM_BLOCK_DATA_SIZE, &info.size);
        refinfo->uniformBlocks.push_back(info);
    }
	hGLSyncFlush();
    refinfo->prog = p;
    return refinfo;
}

void destroyProgramReflectionInfo(hProgramReflectionInfo* p) {
    enqueueRenderResourceDelete([=](){
        glDeleteProgram(p->prog);
        delete p;
    });
}

hShaderParamInfo getParameterInfo(hProgramReflectionInfo* p, const hChar* name) {
    hShaderParamInfo r = {~0u, ~0u, ShaderParamType::Unknown, 0ul};

    auto f = p->paramTable.find(name);
    if (f != p->paramTable.end())
        r = f->second;
    return r;
}

hUint getUniformBlockCount(hProgramReflectionInfo* p) {
    return (hUint)p->uniformBlocks.size();
}

hUniformBlockInfo getUniformBlockInfo(hProgramReflectionInfo* p, hUint i) {
    hUniformBlockInfo info = { nullptr, ~0u, 0 };
    if (i > p->uniformBlocks.size())
        return info;
    return p->uniformBlocks[i];
}

hUint getParameterTypeByteSize(ShaderParamType type) {
	switch(type) {
	case ShaderParamType::Unknown:	return -1;
	case ShaderParamType::Float:	return 4;
	case ShaderParamType::Float2:	return 8;
	case ShaderParamType::Float3:	return 12;
	case ShaderParamType::Float4:	return 16;
	case ShaderParamType::Float22:	return 16;
	case ShaderParamType::Float23:	return 24;
	case ShaderParamType::Float24:	return 32;
	case ShaderParamType::Float32:	return 16;
	case ShaderParamType::Float33:	return 36;
	case ShaderParamType::Float34:	return 48;
	case ShaderParamType::Float42:	return 32;
	case ShaderParamType::Float43:	return 48;
	case ShaderParamType::Float44:	return 64;
	default: 		return 0;
	}
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
    auto* cmd = (hGLClear*)cl->allocCmdMem(Op::Clear, sizeof(hGLClear));
    cmd->colour = colour;
    cmd->depth = depth;
}

hRenderFence* fence(hCmdList* cl) {
	lfds_freelist_element* e;
	hRenderFence* f;
	lfds_freelist_use(fenceFreeList);
	lfds_freelist_pop(fenceFreeList, &e);
	hcAssertMsg(e, "Ran out of fences. This is fatal.");
	lfds_freelist_get_user_data_from_element(e, (void**)&f);

	f->element = e;
	auto* cmd = (hGLFence*)cl->allocCmdMem(Op::Fence, sizeof(hGLFence));
	cmd->fence = f;
	return f;
}

void wait(hRenderFence* fence) {
	hglEnsureTLSContext();
	while (fence->sync == nullptr) {
		Device::ThreadYield();
	}
	hcAssert(glIsSync(fence->sync));
	GLenum result = GL_TIMEOUT_EXPIRED;
	while (result != GL_ALREADY_SIGNALED && result != GL_CONDITION_SATISFIED) {
		//5 Second timeout but we ignore timeouts and wait until all OpenGL commands are processed! 
		result = glClientWaitSync(fence->sync, GL_SYNC_FLUSH_COMMANDS_BIT, GLuint64(5000000000));
	}

	glDeleteSync(fence->sync);
	fence->sync = nullptr;
    hAtomic::LWMemoryBarrier();
	lfds_freelist_push(fenceFreeList, fence->element);
}

void scissorRect(hCmdList* cl, hUint left, hUint top, hUint right, hUint bottom) {
    auto* cmd = (hGLScissor*)cl->allocCmdMem(Op::SetScissor, sizeof(hGLScissor));
    cmd->x = left;
    cmd->y = top;
    cmd->width = right-left;
    cmd->height = bottom-top;
}

void draw(hCmdList* cl, hRenderCall* rc, Primative pt, hUint prims, hUint vtx_offset) {
	auto* cmd = (hGLDraw*)cl->allocCmdMem(Op::Draw, sizeof(hGLDraw));
	cmd->rc = rc;
	cmd->count = prims;
    cmd->vtxOffset = vtx_offset;
	cmd->primType = hglToPrimType(pt, &cmd->count);
}

void flushUnibufferMemoryRange(hCmdList* cl, hUniformBuffer* ub, hUint offset, hUint size) {
	auto* cmd = (hGLUniBufferFlush*)cl->allocCmdMem(Op::UniBufferFlush, sizeof(hGLUniBufferFlush));
	cmd->ub = ub;
	cmd->offset = offset;
	cmd->size = size;
}

void flushVertexBufferMemoryRange(hCmdList* cl, hVertexBuffer* vb, hUint offset, hUint size) {
    if (ft.impl_flushVertexBuffer) {
        auto* cmd = (hGLVertexBufferFlush*)cl->allocCmdMem(Op::VtxBufferFlush, sizeof(hGLVertexBufferFlush));
        cmd->vb = vb;
        cmd->offset = offset;
        cmd->size = size;
    }
}

void call(hCmdList* cl, hCmdList* tocall) {
    auto* cmd = (hGLCall*)cl->allocCmdMem(Op::Call, sizeof(hGLCall));
    cmd->jumpTo=tocall;
    endReturn(tocall);
}

void endReturn(hCmdList* cl) {
    cl->allocCmdMem(Op::Return, 0);
}

void swapBuffers(hCmdList* cl) {
    cl->allocCmdMem(Op::Swap, 0);
}

void submitFrame(hCmdList* cl) {
	auto writeindex = fenceIndex;
	// wait for prev frame to finish
	if (frameFences[fenceIndex]) {
		wait(frameFences[fenceIndex]);
		frameFences[fenceIndex] = nullptr;
	}

	hCmdList* ncl=cl->prev;

	hcAssert(frameFences[fenceIndex] == nullptr);
	frameFences[fenceIndex] = fence(ncl);
	fenceIndex = (fenceIndex+1)%FRAME_COUNT;

    ncl->allocCmdMem(Op::Return, 0);

    flush(cl);

    // Reset & Flip render scratch buffer
    rtmp_frameend();

    if (multiThreadedRenderer == false) {
        renderDoFrame();
    }
}

void flush(hCmdList* cl) {
    lfds_queue_use(cmdListQueue);
    while (lfds_queue_enqueue(cmdListQueue, cl) == 0) {
        Device::ThreadYield();
    }
}

void finish() {
    if (multiThreadedRenderer) {
        auto* cl = createCmdList();
        auto* f = fence(cl);
        endReturn(cl);
        flush(cl);
        wait(f);
    } else {
        
    }
}

}
}
