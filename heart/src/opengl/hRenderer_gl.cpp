/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hTypes.h"
#include "base/hRendererConstants.h"
#include "base/hMemory.h"
#include "base/hStringUtil.h"
#include "core/hSystem.h"
#include "pal/hDeviceThread.h"
#include "threading/hThreadLocalStorage.h"
#include "pal/hSemaphore.h"
#include "render/hIndexBufferFlags.h"
#include "render/hVertexBufferFlags.h"
#include "render/hRenderCallDesc.h"
#include "render/hMipDesc.h"
#include "render/hTextureFormat.h"
#include "render/hUniformBufferFlags.h"
#include "render/hRenderCallDesc.h"
#include "render/hProgramReflectionInfo.h"
#include "render/hRenderPrim.h"
#include "opengl/hRendererOpCodes_gl.h"
#include "cryptoMurmurHash.h"
#include <GL/glew.h>    
#include <GL/gl.h>
#include <SDL.h>
#include <memory>
#include <unordered_map>

namespace Heart {    

class hSystem;

namespace hRenderer {

struct hGLErrorSentry {
    const hChar* file;
    hSize_t line;

    hGLErrorSentry(const hChar* f, hSize_t l) 
        : file(f), line(l){
        //glGetError(); // clear the error if needed
    }

    ~hGLErrorSentry() {
        auto ec = glGetError();
        if (ec != GL_NO_ERROR) {
            const char* errorstr = "Unknown Error";
            switch (ec) {
            case GL_INVALID_ENUM: errorstr = "Invalid Enum"; break;
            case GL_INVALID_VALUE: errorstr = "Invalid Value"; break;
            case GL_INVALID_OPERATION: errorstr = "Invalid Operation"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: errorstr = "Invalid Framebuffer operation"; break;
            case GL_OUT_OF_MEMORY: errorstr = "Out of Memory"; break;
            default: break;
            }
            hcPrintf("%s:%d: OpenGL Error %s", file, line, errorstr);
        }
    }
};

#ifdef HEART_DEBUG
#   define hGLErrorScope() Heart::hRenderer::hGLErrorSentry gl_error_sentry__blah_blah__(__FILE__, __LINE__)
#else
#   define hGLErrorScope()
#endif

struct hRenderCall {
    enum Flag : hUint8 {
        VAOBound = 0x80,
    };

    hRenderCall() 
        : size_(0)
        , opCodes_(nullptr) 
        , flags(0) {
    }
    hGLRCHeader header_;
    GLuint program_;
    GLuint vao;
    hVertexBuffer* vtxBuf_;
    hUint size_;
    hByte* opCodes_;
    GLuint* samplers_;
    hUint8  flags;
};

struct hGLObjectBase {
	hGLObjectBase() 
		: flags(0) {

	}

	enum class Flag : hUint8 {
		RenderThreadBound = 0x80, // Set on successful 
		ValidName		  = 0x40,
	};

	GLuint  name;
	hUint32 flags;
};

struct hIndexBuffer : hGLObjectBase {
    hUint32 indices;
    hUint32 createFlags_;
};

struct hVertexBuffer : hGLObjectBase {
    hUint32 elementCount_;
    hUint32 elementSize_;
    hUint32 createFlags_;
};

struct hUniformBuffer : hGLObjectBase {
    hUint   size_;
    hUint32 createFlags_;
};

struct hTextureBase : hGLObjectBase {
    GLenum  target_;
    GLuint  internalFormat_;
    GLuint  format_;
};

struct hTexture2D : hTextureBase {

};

struct hCmdList {
    static const hUint MinCmdBlockSize = 64*1024;
    hCmdList() {
        prev = next = this;
        cmds = (hByte*)hMalloc(MinCmdBlockSize);
        cmdsSize = 0;
        cmdsReserve = MinCmdBlockSize;
    }
    ~hCmdList() {
        hFree(cmds);
        cmds = nullptr;
    }
    hByte* allocCmdMem(Op opcode, hUint s) {
        auto needed = s+OpCodeSize;
        if ((cmdsReserve - cmdsSize) < needed) {
            hRealloc(cmds, cmdsReserve+MinCmdBlockSize);
            cmdsReserve += MinCmdBlockSize;
        }
        hByte* r=cmds+cmdsSize;
        cmdsSize+=needed;
        *((Op*)r) = opcode;
        return (r+OpCodeSize);
    }
    void clear() {
        cmdsSize = 0;
    }

    hCmdList* prev, *next;
    hByte* cmds;
    hUint  cmdsSize;
    hUint  cmdsReserve;


};

namespace RenderPrivate {
	hBool				multiThreadedRenderer = false;
    SDL_Window*         window_;
    SDL_GLContext       context_;
    hThread             renderThread_;

	SDL_GLContext       mtContext_ = nullptr;
    hConditionVariable  rtComsSignal;
    hMutex              rtMutex;

    hSize_t             tlsContext_;

    hCmdList*           rtCmdLists_[2];
    hSemaphore          rtFrameSubmitSema;
    hSemaphore          rtFrameCompleteSema;
    hMutex              rtFrameMtx;
    hUint               rtFrameIdx;
}

// !!JM TODO: Improve these, they are placeholder & rtmp_free may be unnessary (i.e. we always free at frame end) 
static void* rtmp_malloc(hSize_t size) {
    return hMalloc(size);
}

static void rtmp_free(void* ptr) {
    hFree(ptr);
}

static void hglEnsureTLSContext() {
    using namespace RenderPrivate;
    SDL_GLContext ctx = (SDL_GLContext)TLS::getKeyValue(tlsContext_);
	if (!ctx && multiThreadedRenderer) {
		SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
		SDL_GL_MakeCurrent(window_, mtContext_);
        ctx = SDL_GL_CreateContext(window_);
        TLS::setKeyValue(tlsContext_, ctx);
        SDL_GL_MakeCurrent(window_, ctx);
		if (GL_ARB_debug_output) {
			glDebugMessageCallbackARB([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
				hcPrintf("OpenGL Debug Message: %s", message);
			}, nullptr);
		}
    }
}

static SDL_GLContext hglTLSMakeCurrent() {
    using namespace RenderPrivate;
    hglEnsureTLSContext();
    return (SDL_GLContext)TLS::getKeyValue(tlsContext_);
}

static void hglReleaseTLSContext() {
    using namespace RenderPrivate;

    SDL_GLContext ctx = (SDL_GLContext)TLS::getKeyValue(tlsContext_);
	if (ctx) {
        SDL_GL_DeleteContext(ctx);
        TLS::setKeyValue(tlsContext_, nullptr);
    }
}

static void hGLSyncFlush() {
	GLsync fenceId = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 ); 
	GLenum result = GL_TIMEOUT_EXPIRED; 
	while(result != GL_ALREADY_SIGNALED && result != GL_CONDITION_SATISFIED) { 
		//5 Second timeout but we ignore timeouts and wait until all OpenGL commands are processed! 
		result = glClientWaitSync(fenceId, GL_SYNC_FLUSH_COMMANDS_BIT, GLuint64(5000000000));
	}
}

static void renderDoFrame() {
	using namespace RenderPrivate;

	rtFrameSubmitSema.Wait();
	rtFrameMtx.Lock();
	hCmdList* fcmds = rtCmdLists_[rtFrameIdx];
	hCmdList* cmds = rtCmdLists_[rtFrameIdx];
	rtCmdLists_[rtFrameIdx] = nullptr;
	rtFrameIdx = (rtFrameIdx + 1) & 0x1;
	rtFrameMtx.Unlock();

	while (cmds) {
		cmds = cmds->next;
		for (hByte* cmdptr = cmds->cmds, *n = cmds->cmds + cmds->cmdsSize; cmdptr < n;) {
			Op opcode = *((hRenderer::Op*)cmdptr);
			cmdptr += OpCodeSize;
			switch (opcode) {
			case Op::NoOp: break;
			case Op::Clear: {
				hGLErrorScope();
				hGLClear* c = (hGLClear*)cmdptr;
				glClearColor(c->colour.r_, c->colour.g_, c->colour.b_, c->colour.a_);
				glClear(GL_COLOR_BUFFER_BIT);
				cmdptr += sizeof(hGLClear);
			} break;
			case Op::Draw: {
				hGLErrorScope();
#define hGetArgs(x) (x*)args; args+=sizeof(x)
#define hGetArgsN(x,c) (x*)args; args+=(sizeof(x)*(c))
				hGLDraw* c = (hGLDraw*)cmdptr;
				hRenderCall* rc = c->rc;
				hByte* args = rc->opCodes_;
				cmdptr += sizeof(hGLDraw);

				if (rc->header_.blend) {
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
				//sampler states
				if (rc->header_.samplerCount_) {
					auto* samplers = hGetArgsN(hGLSampler, rc->header_.samplerCount_);
					for (hUint8 i = 0, n = rc->header_.samplerCount_; i < n; ++i) {
						glBindSampler(samplers[i].index, samplers[i].samplerObj);
					}
				}
				//textures
				if (rc->header_.textureCount_) {
					auto* textures = hGetArgsN(hGLTexture2D, rc->header_.textureCount_);
					for (hUint8 i = 0, n = rc->header_.textureCount_; i < n; ++i) {
						glActiveTexture(textures[i].index_);
						glBindTexture(textures[i].tex_->target_, textures[i].tex_->name);
					}
				}
				// uniform buffers
				if (rc->header_.uniBufferCount_) {
					auto* ubs = hGetArgsN(hGLUniformBuffer, rc->header_.uniBufferCount_);
					for (hUint8 i = 0, n = rc->header_.uniBufferCount_; i < n; ++i) {
						glBindBuffer(ubs[i].index_, ubs[i].ub_->name);
					}
				}

				glUseProgram(rc->program_);

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
						glVertexAttribPointer(va[i].index_, va[i].size_, va[i].type_, GL_FALSE, va[i].stride_, va[i].pointer_);
					}

					rc->flags |= hRenderCall::VAOBound;
				}
				else {
					hGLErrorScope();
					glBindVertexArray(rc->vao);
				}

					{
						hGLErrorScope();
						glDrawArrays(c->primType, 0, c->count);
					}
				break;
#undef hGetArgs
#undef hGetArgsN
			}
			case Op::Swap: {
				hGLErrorScope();
				SDL_GL_SwapWindow(window_);
			} break;
			}
		}

		if (cmds == fcmds)
			break;
	}

	rtFrameCompleteSema.Post();
}

hUint32 renderThreadMain(void* param) {
    using namespace RenderPrivate;

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
    using namespace RenderPrivate;

	multiThreadedRenderer = !!hConfigurationVariables::getCVarUint("gl.multithreaded", 0);

    tlsContext_ = TLS::createKey([](void* ctx) {
        SDL_GL_DeleteContext(ctx);
    });

    rtFrameSubmitSema.Create(0, 1);
    rtFrameCompleteSema.Create(1, 1);
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
		glDebugMessageCallbackARB([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
			hcPrintf("OpenGL Debug Message: %s", message);
		}, nullptr);
	}

	if (multiThreadedRenderer == true) {
		SDL_GL_MakeCurrent(window_, nullptr);

		renderThread_.create("OpenGL Render Thread", hThread::PRIORITY_NORMAL, hFUNCTOR_BINDSTATIC(hThreadFunc, renderThreadMain), nullptr);

		rtMutex.Lock();
		rtComsSignal.wait(&rtMutex);
		rtMutex.Unlock();

		hglTLSMakeCurrent();
	}
	
    //check for required extentions
    // !!JM TODO: handle failure better than just asserting...
    if (!GLEW_VERSION_3_3) {
        hcAssertFailMsg("OpenGL 3.3 is required but not supported.");
    }
    if (!GLEW_EXT_texture_compression_s3tc) {
        hcAssertFailMsg("GL_EXT_texture_compression_s3tc is required but not found.");
    }
    if (!GLEW_EXT_texture_sRGB) {
        hcAssertFailMsg("GL_EXT_texture_sRGB is required but not found.");    
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void destroy() {
    using namespace RenderPrivate;


    hglReleaseTLSContext();
    context_=nullptr;
}

GLuint hglProfileToType(hShaderProfile profile) {
    switch (profile) {
    case eShaderProfile_vs4_0: return GL_VERTEX_SHADER;
    case eShaderProfile_vs4_1: return GL_VERTEX_SHADER;
    case eShaderProfile_vs5_0: return GL_VERTEX_SHADER;
    case eShaderProfile_ps4_0: return GL_FRAGMENT_SHADER;
    case eShaderProfile_ps4_1: return GL_FRAGMENT_SHADER;
    case eShaderProfile_ps5_0: return GL_FRAGMENT_SHADER;
    case eShaderProfile_gs4_0: return GL_GEOMETRY_SHADER;
    case eShaderProfile_gs4_1: return GL_GEOMETRY_SHADER;
    case eShaderProfile_gs5_0: return GL_GEOMETRY_SHADER;
    case eShaderProfile_cs4_0: //return ;
    case eShaderProfile_cs4_1: //return ;
    case eShaderProfile_cs5_0: //return ;
    case eShaderProfile_hs5_0: //return ;
    case eShaderProfile_ds5_0: //return ;
    default: return GL_INVALID_VALUE;
    }
}

static GLenum hglToPrimType(Primative pt, hUint* count) {
	switch (pt) {
	case Primative::Triangles: (*count) *= 3; return GL_TRIANGLES;
	case Primative::TriangleStrip: (*count) += 2; return GL_TRIANGLE_STRIP;
	default: return GL_INVALID_VALUE;
	}
}

struct hShaderStage {
    GLuint  shaderObj_;
};

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
    auto fn = [=]() {
        glDeleteShader(shader->shaderObj_);
        delete shader;
    };
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
    auto fn = [=]() {
        glDeleteShader(ib->name);
        delete ib;
    };
}

hVertexBuffer*  createVertexBuffer(const void* data, hUint32 elementsize, hUint32 elementcount, hUint32 flags) {
    hglEnsureTLSContext();
    hGLErrorScope();
    GLuint bname;
    glGenBuffers(1, &bname);
    if (!bname) {
        return nullptr;
    }
    glBindBuffer(GL_ARRAY_BUFFER, bname);
    GLuint size = elementsize * elementcount;
    glBufferData(GL_ARRAY_BUFFER, size, data, (flags & (hUint32)hVertexBufferFlags::DynamicBuffer) ?  GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	hGLSyncFlush();
    auto* vb = new hVertexBuffer();
    vb->name = bname;
    vb->elementCount_ = elementcount;
    vb->elementSize_ = elementsize;
    vb->createFlags_ = flags;
    return vb;
}

void  destroyVertexBuffer(hVertexBuffer* vb) {
    //deletes are deferred, render thread will do it
    auto fn = [=]() {
        glDeleteShader(vb->name);
        delete vb;
    };
}

hTexture2D*  createTexture2D(hUint32 levels, hMipDesc* initdata, hTextureFormat format, hUint32 flags) {
    hglEnsureTLSContext();
    hGLErrorScope();
    GLuint tname;
    glGenTextures(1, &tname);
    glActiveTexture(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS-1);
    glBindTexture(GL_TEXTURE_2D, tname);

    GLuint fmt = GL_INVALID_VALUE;
    GLuint intfmt = GL_INVALID_VALUE;
    GLuint type = GL_INVALID_VALUE;
    hBool compressed = false;
    switch(format) {
    case hTextureFormat::RGBA8_unorm:      intfmt=GL_RGBA;                                fmt=GL_RGBA; type=GL_UNSIGNED_BYTE; compressed = false; break; 
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

    if (compressed) {
        for (auto i=0u; i<levels; ++i) {
            glCompressedTexImage2D(GL_TEXTURE_2D, i, intfmt, initdata[i].width, initdata[i].height, 0, initdata[i].size, initdata[i].data);
        }
    } else {
        for (auto i=0u; i<levels; ++i) {
            glTexImage2D(GL_TEXTURE_2D, i, intfmt, initdata[i].width, initdata[i].height, 0, fmt, type, initdata[i].data);
        }
    }
	hGLSyncFlush();
    auto* t = new hTexture2D();
    t->name = tname;
    t->target_ = GL_TEXTURE_2D;
    t->internalFormat_ = intfmt;
    t->format_ = fmt;
    return t;
}

void  destroyTexture2D(hTexture2D* t) {
    auto fn = [=]() {
        glDeleteTextures(1, &t->name);
        delete t;
    };
}

hUniformBuffer* createUniformBuffer(const void* initdata, hUint size, hUint32 flags) {
    GLuint ubname;
    glGenBuffers(1, &ubname);
    glBindBuffer(GL_UNIFORM_BUFFER, ubname);
    glBufferData(GL_UNIFORM_BUFFER, size, initdata, (flags & (hUint32)hUniformBufferFlags::Dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	hGLSyncFlush();
    auto* ub = new hUniformBuffer();
    ub->name=ubname;
    ub->size_=size;
    ub->createFlags_=flags;
    return ub;
}

void destroyUniformBuffer(hUniformBuffer* ub) {
    auto fn = [=]() {
        glDeleteBuffers(1, &ub->name);
        delete ub;
    };
}

hRenderCall* createRenderCall(const hRenderCallDesc& rcd) {
    hglEnsureTLSContext();
    auto* rc = new hRenderCall();
    GLint p;

    rc->size_ = 0;
    rc->opCodes_ = nullptr;
    auto& header = rc->header_;

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
    auto* ubary = (GLchar**)hAlloca(ubtotal*sizeof(GLchar*));
    auto* ubnames = (hChar*)hAlloca(ubtotal*ublimit);
    //auto* ubhashes = (hUint32*)hAlloca(ubtotal*sizeof(hUint32));

    for (auto i=0,n=ubtotal; i<n; ++i) {
        auto olen = 0;
        ubary[i] = ubnames+(i*ublimit);
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
    auto* aary = (GLchar**)hAlloca(atotal*sizeof(GLchar*));
    auto* anames = (hChar*)hAlloca(atotal*alimit);
    auto* atypes = (GLenum*)hAlloca(atotal*sizeof(GLenum));
    auto* asizes = (GLint*)hAlloca(atotal*sizeof(GLint));
    auto* aloc = (hUint32*)hAlloca(atotal*sizeof(hUint32));

    for (auto i=0,n=atotal; i<n; ++i) {
        auto olen = 0;
        aary[i] = anames+(i*alimit);
        glGetActiveAttrib(rc->program_, i, alimit, &olen, asizes+i, atypes+i, aary[i]);
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
    auto* uary = (GLchar**)hAlloca(utotal*sizeof(GLchar*));
    auto* unames = (hChar*)hAlloca(utotal*ulimit);
    auto* utypes = (GLenum*)hAlloca(utotal*sizeof(GLenum));
    auto* usizes = (GLint*)hAlloca(utotal*sizeof(GLint));
    //auto* uhashes = (hUint32*)hAlloca(utotal*sizeof(hUint32));

    for (auto i=0,n=utotal; i<n; ++i) {
        auto olen = 0;
        uary[i] = unames+(i*ulimit);
        glGetActiveUniform(rc->program_, i, ulimit, &olen, usizes+i, utypes+i, uary[i]);
        //cyMurmurHash3_x86_32(uary[i], olen, hGetMurmurHashSeed(), uhashes+i);
        // !!JM TODO: more types handled?
        if (utypes[i] == GL_SAMPLER_1D || utypes[i] == GL_SAMPLER_2D || utypes[i] == GL_SAMPLER_3D) {
            ++header.samplerCount_;
            ++header.textureCount_;
        }
    }

    auto getuniformindex = [&](const hStringID& id) -> hUint {
        for (auto i=0, n=utotal; i<n; ++i) {
            if (hStrCmp(uary[i], id.c_str())==0) return i;
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
        rc->opCodes_ = (hByte*)hRealloc(rc->opCodes_, ns);
        auto* glblend = (hGLBlend*)(rc->opCodes_+rc->size_);
        glblend->func = blendfunctogl(rcd.blend_.blendOp_);
        glblend->src = blendoptogl(rcd.blend_.srcBlend_);
        glblend->dest = blendoptogl(rcd.blend_.destBlend_);
        rc->size_ = ns;
    }
    // seperate alpha blend state
    if (header.seperateAlpha) {
        auto ns = (hUint32)(rc->size_+sizeof(hGLBlend));
        rc->opCodes_ = (hByte*)hRealloc(rc->opCodes_, ns);
        auto* glblend = (hGLBlend*)(rc->opCodes_+rc->size_);
        glblend->func = blendfunctogl(rcd.blend_.blendOpAlpha_);
        glblend->src = blendoptogl(rcd.blend_.srcBlendAlpha_);
        glblend->dest = blendoptogl(rcd.blend_.destBlendAlpha_);
        rc->size_ = ns;
    }

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
        rc->opCodes_ = (hByte*)hRealloc(rc->opCodes_, ns);
        auto* gldepth = (hGLDepth*)(rc->opCodes_+rc->size_);
        gldepth->func = comparetogl(rcd.depthStencil_.depthFunc_);
        gldepth->mask = rcd.depthStencil_.depthWriteMask_;
        rc->size_=ns;
    }
    //depth bais
    if (header.depthBais) {
        auto ns = (hUint32)(rc->size_+sizeof(hGLDepthBais));
        rc->opCodes_ = (hByte*)hRealloc(rc->opCodes_, ns);
        auto* gldepthbais = (hGLDepthBais*)(rc->opCodes_+rc->size_);
        gldepthbais->depthBias_ = rcd.rasterizer_.depthBias_;
        gldepthbais->depthBiasClamp_ = rcd.rasterizer_.depthBiasClamp_;
        gldepthbais->slopeScaledDepthBias_ = rcd.rasterizer_.slopeScaledDepthBias_;
        gldepthbais->depthClipEnable_ = rcd.rasterizer_.depthClipEnable_;
        rc->size_=ns;
    }

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
        rc->opCodes_ = (hByte*)hRealloc(rc->opCodes_, ns);
        auto* glstencil = (hGLStencil*)(rc->opCodes_+rc->size_);
        glstencil->readMask_ = rcd.depthStencil_.stencilReadMask_;
        glstencil->writeMask_ = rcd.depthStencil_.stencilWriteMask_;
        glstencil->ref_ = rcd.depthStencil_.stencilRef_;
        glstencil->failOp_ = stenciloptogl(rcd.depthStencil_.stencilFailOp_);
        glstencil->depthFailOp_ = stenciloptogl(rcd.depthStencil_.stencilDepthFailOp_);
        glstencil->passOp_ = stenciloptogl(rcd.depthStencil_.stencilPassOp_);
        glstencil->func_ = comparetogl(rcd.depthStencil_.stencilFunc_);
        rc->size_=ns;
    }

    
    // sampler states
    auto filtertogl = [] (proto::renderstate::SamplerState a) -> GLint {
        switch(a) {
        case proto::renderstate::point       : return GL_NEAREST;
        case proto::renderstate::linear      : return GL_LINEAR;
        case proto::renderstate::anisotropic : return GL_LINEAR;
        }
        return GL_INVALID_VALUE;
    };
    auto minfiltertogl = [] (proto::renderstate::SamplerState a) -> GLint {
        switch(a) {
        case proto::renderstate::point       : return GL_NEAREST_MIPMAP_NEAREST;
        case proto::renderstate::linear      : return GL_LINEAR_MIPMAP_LINEAR;
        case proto::renderstate::anisotropic : return GL_LINEAR_MIPMAP_LINEAR;
        }
        return GL_INVALID_VALUE;
    };
    auto bordertogl = [](proto::renderstate::SamplerBorder a) -> GLint {
        switch(a) {
        case proto::renderstate::wrap  : return GL_REPEAT;
        case proto::renderstate::mirror: return GL_MIRRORED_REPEAT;
        case proto::renderstate::clamp : return GL_CLAMP_TO_EDGE;
        case proto::renderstate::border: return GL_CLAMP_TO_BORDER;
        }
        return GL_INVALID_VALUE;
    };
    if (header.samplerCount_) {
        auto ns = (hUint32)(rc->size_+(sizeof(GLuint)*header.samplerCount_));
        rc->opCodes_ = (hByte*)hRealloc(rc->opCodes_, ns);
        auto* samplers = (hGLSampler*)(rc->opCodes_+rc->size_);
        auto* wsamplers = samplers;
        for (auto i=0u, n=hRenderCallDesc::samplerStateMax_; i<n && !rcd.samplerStates_[i].name_.is_default(); ++i) {
            auto si = getuniformindex(rcd.samplerStates_[i].name_);
            if (si != ~0u) {
                wsamplers->index = si;
                glGenSamplers(1, &wsamplers->samplerObj);
                auto minfilter = minfiltertogl(rcd.samplerStates_[i].sampler_.filter_);
                auto filter = filtertogl(rcd.samplerStates_[i].sampler_.filter_);
                auto wraps = bordertogl(rcd.samplerStates_[i].sampler_.addressU_);
                auto wrapt = bordertogl(rcd.samplerStates_[i].sampler_.addressV_);
                auto wrapr = bordertogl(rcd.samplerStates_[i].sampler_.addressW_);
                glSamplerParameteriv(wsamplers->samplerObj, GL_TEXTURE_WRAP_S, &wraps);
                glSamplerParameteriv(wsamplers->samplerObj, GL_TEXTURE_WRAP_T, &wrapt);
                glSamplerParameteriv(wsamplers->samplerObj, GL_TEXTURE_WRAP_R, &wrapr);
                glSamplerParameteriv(wsamplers->samplerObj, GL_TEXTURE_MIN_FILTER, &minfilter);
                glSamplerParameteriv(wsamplers->samplerObj, GL_TEXTURE_MAG_FILTER, &filter);
                glSamplerParameterfv(wsamplers->samplerObj, GL_TEXTURE_MIN_LOD, &rcd.samplerStates_[i].sampler_.minLOD_);
                glSamplerParameterfv(wsamplers->samplerObj, GL_TEXTURE_MAX_LOD, &rcd.samplerStates_[i].sampler_.maxLOD_);
                glSamplerParameterfv(wsamplers->samplerObj, GL_TEXTURE_BORDER_COLOR, (hFloat*)&rcd.samplerStates_[i].sampler_.borderColour_);
                ++wsamplers;
            }
        }
        rc->size_=ns;    
    }

    //textures
    if (header.textureCount_) {
        auto ns = (hUint32)(rc->size_+(sizeof(hGLTexture2D)*header.textureCount_));
        rc->opCodes_ = (hByte*)hRealloc(rc->opCodes_, ns);
        auto* textures = (hGLTexture2D*)(rc->opCodes_+rc->size_);
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

    // uniform buffers
    if (header.uniBufferCount_) {
        auto ns = (hUint32)(rc->size_+(sizeof(hGLUniformBuffer)*header.uniBufferCount_));
        rc->opCodes_ = (hByte*)hRealloc(rc->opCodes_, ns);
        auto* buffers = (hGLUniformBuffer*)(rc->opCodes_+rc->size_);
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

    // index
    if (header.index) {
        auto ns = (hUint32)(rc->size_+(sizeof(hIndexBuffer*)));
        rc->opCodes_ = (hByte*)hRealloc(rc->opCodes_, ns);
        auto** ib = (hIndexBuffer**)(rc->opCodes_+rc->size_);            
        *ib = rcd.indexBuffer_;
        rc->size_=ns;    
    }

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
        rc->opCodes_ = (hByte*)hRealloc(rc->opCodes_, ns);
        auto* attribs = (hGLVtxAttrib*)(rc->opCodes_+rc->size_);
        for (auto i=0u, n=hRenderCallDesc::vertexLayoutMax_; i<n && !rcd.vertexLayout_[i].bindPoint_.is_default(); ++i) {
            auto bindpoint = getattribindex(rcd.vertexLayout_[i].bindPoint_);
            if (bindpoint != ~0u) {
				hcAssertMsg(bindpoint < header.vtxAttCount_, "Vertex input index is too high");
                attribs[bindpoint].index_=bindpoint;
                attribs[bindpoint].size_=rcd.vertexLayout_[i].elementCount_;
                attribs[bindpoint].type_=inputtypetogl(rcd.vertexLayout_[i].type_);
                attribs[bindpoint].pointer_=(void*)rcd.vertexLayout_[i].offset_;
                attribs[bindpoint].stride_=rcd.vertexLayout_[i].stride_;
            }
        }
        rc->size_=ns;
    }
	hGLSyncFlush();
    return rc;
}

struct hProgramReflectionInfo {
    typedef std::unordered_map<hUint32, hShaderParamInfo> hParamHash;

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
        auto hash = 0u;
        cyMurmurHash3_x86_32(uniname, olen, hGetMurmurHashSeed(), hashes+i);
        hShaderParamInfo info = { (hUint)blockindex, (hUint)blockoffset, mapGLParamType(type), (hUint)size };
        refinfo->paramTable[hashes[i]] = info;
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

}

hShaderParamInfo getParameterInfo(hProgramReflectionInfo* p, const hChar* name) {
    hShaderParamInfo r = {~0u, ~0u, ShaderParamType::Unknown, 0ul};
    auto hash = 0u;
    cyMurmurHash3_x86_32(name, hStrLen(name), hGetMurmurHashSeed(), &hash);
    auto f = p->paramTable.find(hash);
    if (f != p->paramTable.end())
        r = f->second;
    return r;
}

hUint getUniformatBlockCount(hProgramReflectionInfo* p) {
    return (hUint)p->uniformBlocks.size();
}

hUniformBlockInfo getUniformatBlockInfo(hProgramReflectionInfo* p, hUint i) {
    hUniformBlockInfo info = { nullptr, ~0u, 0 };
    if (i > p->uniformBlocks.size())
        return info;
    return p->uniformBlocks[i];
}

hCmdList* createCmdList() {
    return new hCmdList();
}

void      destroyCmdList(hCmdList*) {

}

void      clearCmdList(hCmdList* cl) {
    cl->clear();
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

void draw(hCmdList* cl, hRenderCall* rc, Primative pt, hUint prims) {
	auto* cmd = (hGLDraw*)cl->allocCmdMem(Op::Draw, sizeof(hGLDraw));
	cmd->rc = rc;
	cmd->count = prims;
	cmd->primType = hglToPrimType(pt, &cmd->count);
}

void swapBuffers(hCmdList* cl) {
    cl->allocCmdMem(Op::Swap, 0);
}

void submitFrame(hCmdList* cl) {
    using namespace RenderPrivate;

    rtFrameCompleteSema.Wait();
    rtFrameMtx.Lock();
    hcAssert(rtCmdLists_[rtFrameIdx] == nullptr);
    rtCmdLists_[rtFrameIdx] = cl;
    rtFrameMtx.Unlock();
    rtFrameSubmitSema.Post();

	if (multiThreadedRenderer == false) {
		renderDoFrame();
	}
}
   
}
}
