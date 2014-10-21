/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hTypes.h"
#include "base/hRendererConstants.h"
#include "base/hMemory.h"
#include "core/hSystem.h"
#include "pal/hDeviceThread.h"
#include "threading/hThreadLocalStorage.h"
#include "render/hIndexBufferFlags.h"
#include "render/hVertexBufferFlags.h"
#include "render/hRenderCallDesc.h"
#include "render/hMipDesc.h"
#include "render/hTextureFormat.h"
#include "render/hUniformBufferFlags.h"
#include "render/hRenderCallDesc.h"
#include "opengl/hRendererOpCodes_gl.h"
#include "cryptoMurmurHash.h"
#include <GL/glew.h>    
#include <GL/gl.h>
#include <SDL.h>

namespace Heart {    

class hSystem;

namespace hRenderer {

namespace RenderPrivate {
    SDL_Window*         window_;
    SDL_GLContext       context_;
    hThread             renderThread_;

    SDL_GLContext       mtContext_;
    hConditionVariable  rtComsSignal;
    hMutex              rtMutex;

    hSize_t             tlsContext_;
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
    if (!ctx) {
        ctx = SDL_GL_CreateContext(window_);
        TLS::setKeyValue(tlsContext_, ctx);
        SDL_GL_MakeCurrent(window_, ctx);
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

hUint32 renderThreadMain(void* param) {
    using namespace RenderPrivate;

    context_ = hglTLSMakeCurrent();
    glewExperimental = GL_TRUE;
    GLenum result = glewInit();
    if(result != GLEW_OK) {
        hcPrintf("glewInit() error %d [%s]", result, glewGetErrorString(result));
    }

    rtComsSignal.signal();

    int frame = 0;
    for (;;) {
        glClearColor(0.0, 0.0, ((frame/60)&1) ? 1.0f : 0.0f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(window_);
        ++frame;
    }

    return 0;
}

void create(hSystem* system, hUint32 width, hUint32 height, hUint32 bpp, hFloat shaderVersion, hBool fullscreen, hBool vsync) {
    using namespace RenderPrivate;

    tlsContext_ = TLS::createKey([](void* ctx) {
        SDL_GL_DeleteContext(ctx);
    });

    window_ = system->getSDLWindow();
    renderThread_.create("OpenGL Render Thread", hThread::PRIORITY_NORMAL, hFUNCTOR_BINDSTATIC(hThreadFunc, renderThreadMain), nullptr);

    rtComsSignal.wait(&rtMutex);

    mtContext_ = hglTLSMakeCurrent();

    //check for required extentions
    // !!JM TODO: handle failure better than just asserting...
    if (!GLEW_EXT_texture_compression_s3tc) {
        hcAssertFailMsg("GL_EXT_texture_compression_s3tc is required but not found.");
    }
    if (!GL_EXT_texture_sRGB) {
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

struct hShaderStage {
    GLuint  shaderObj_;
};

hShaderStage* createShaderStage(const hChar* shaderProg, hUint32 len, hShaderType type) {
    return nullptr;
}

hShaderStage* compileShaderStageFromSource(const hChar* shaderProg, hUint32 len, const hChar* entry, hShaderProfile profile) {
    hglEnsureTLSContext();
    auto gls = glCreateShader(hglProfileToType(profile));
    if (!gls)
        return nullptr;
    auto gllen = (GLint)len;
    glShaderSource(gls, 1, &shaderProg, &gllen);
    glCompileShader(gls);
    // to check...but should we defer this until as late as possible i.e. RenderCall create time ???
    GLint status;
    glGetShaderiv(gls, GL_COMPILE_STATUS, &status);
    hcAssert(status == GL_TRUE);
    

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

struct hIndexBuffer {
    GLuint  name_;
    hUint32 indices;
    hUint32 createFlags_;
};

hIndexBuffer* createIndexBuffer(const void* data, hUint32 nIndices, hUint32 flags) {
    hglEnsureTLSContext();
    GLuint bname;
    glGenBuffers(1, &bname);
    if (!bname) {
        return nullptr;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bname);
    GLuint size = nIndices * (flags & (hUint32)hIndexBufferFlags::DwordIndices ? 4 : 2);
    glBufferData(GL_ARRAY_BUFFER, size, data, flags & (hUint32)hIndexBufferFlags::DynamicBuffer);
    auto* ib = new hIndexBuffer();
    ib->name_ = bname;
    ib->indices = nIndices;
    ib->createFlags_ = flags;
    return ib;
}

void destroyIndexBuffer(hIndexBuffer* ib) {
    //deletes are deferred, render thread will do it
    auto fn = [=]() {
        glDeleteShader(ib->name_);
        delete ib;
    };
}

struct hVertexBuffer {
    GLuint  name_;
    hUint32 elementCount_;
    hUint32 elementSize_;
    hUint32 createFlags_;
};

hVertexBuffer*  createVertexBuffer(const void* data, hUint32 elementsize, hUint32 elementcount, hUint32 flags) {
    hglEnsureTLSContext();
    GLuint bname;
    glGenBuffers(1, &bname);
    if (!bname) {
        return nullptr;
    }
    glBindBuffer(GL_ARRAY_BUFFER, bname);
    GLuint size = elementsize * elementcount;
    glBufferData(GL_ARRAY_BUFFER, size, data, flags & (hUint32)hVertexBufferFlags::DynamicBuffer);
    auto* vb = new hVertexBuffer();
    vb->name_ = bname;
    vb->elementCount_ = elementcount;
    vb->elementSize_ = elementsize;
    vb->createFlags_ = flags;
    return vb;
}

void  destroyVertexBuffer(hVertexBuffer* vb) {
    //deletes are deferred, render thread will do it
    auto fn = [=]() {
        glDeleteShader(vb->name_);
        delete vb;
    };
}

struct hTexture2D {
    GLuint  name_;
    GLenum  target_;
    GLuint  internalFormat_;
    GLuint  format_;
};

hTexture2D*  createTexture2D(hUint32 levels, hMipDesc* initdata, hTextureFormat format, hUint32 flags) {
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

    auto* t = new hTexture2D();
    t->name_ = tname;
    t->target_ = GL_TEXTURE_2D;
    t->internalFormat_ = intfmt;
    t->format_ = fmt;
    return t;
}

void  destroyTexture2D(hTexture2D* t) {
    auto fn = [=]() {
        glDeleteTextures(1, &t->name_);
        delete t;
    };
}

struct hUniformBuffer {
    GLuint  name_;
    hUint   size_;
    hUint32 createFlags_;
};

hUniformBuffer* createUniformBuffer(const void* initdata, hUint size, hUint32 flags) {
    GLuint ubname;
    glGenBuffers(1, &ubname);
    glBindBuffer(GL_UNIFORM_BUFFER, ubname);
    glBufferData(GL_UNIFORM_BUFFER, size, initdata, (flags & (hUint32)hUniformBufferFlags::Dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

    auto* ub = new hUniformBuffer();
    ub->name_=ubname;
    ub->size_=size;
    ub->createFlags_=flags;
    return ub;
}

void destroyUniformBuffer(hUniformBuffer* ub) {
    auto fn = [=]() {
        glDeleteBuffers(1, &ub->name_);
        delete ub;
    };
}

struct hRenderCall {
    hRenderCall() 
        : size_(0)
        , opCodes_(nullptr) {
    }
    GLuint program_;
    hUint size_;
    void* opCodes_;
};

hRenderCall* createRenderCall(const hRenderCallDesc& rcd) {
    auto* rc = new hRenderCall();
    GLint p;

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
    auto* ubhashes = (hUint32*)hAlloca(ubtotal*sizeof(hUint32));

    for (auto i=0,n=ubtotal; i<n; ++i) {
        auto olen = 0;
        ubary[i] = ubnames+(i*ublimit);
        glGetActiveUniformBlockName(rc->program_, i, ublimit, &olen, ubary[i]);
        cyMurmurHash3_x86_32(ubary[i], olen, hGetMurmurHashSeed(), ubhashes+i);
    }

    auto alimit = 0;
    auto atotal = 0;
    glGetProgramiv(rc->program_, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &alimit);
    glGetProgramiv(rc->program_, GL_ACTIVE_ATTRIBUTES, &atotal);
    auto* aary = (GLchar**)hAlloca(atotal*sizeof(GLchar*));
    auto* anames = (hChar*)hAlloca(atotal*alimit);
    auto* atypes = (GLenum*)hAlloca(atotal*sizeof(GLenum));
    auto* asizes = (GLint*)hAlloca(atotal*sizeof(GLint));
    auto* ahashes = (hUint32*)hAlloca(atotal*sizeof(hUint32));

    for (auto i=0,n=atotal; i<n; ++i) {
        auto olen = 0;
        aary[i] = anames+(i*alimit);
        glGetActiveAttrib(rc->program_, i, alimit, &olen, asizes+i, atypes+i, aary[i]);
        cyMurmurHash3_x86_32(aary[i], olen, hGetMurmurHashSeed(), ahashes+i);
    }

    auto ulimit = 0;
    auto utotal = 0;
    glGetProgramiv(rc->program_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &ulimit);
    glGetProgramiv(rc->program_, GL_ACTIVE_UNIFORMS, &utotal);
    auto* uary = (GLchar**)hAlloca(utotal*sizeof(GLchar*));
    auto* unames = (hChar*)hAlloca(utotal*ulimit);
    auto* utypes = (GLenum*)hAlloca(utotal*sizeof(GLenum));
    auto* usizes = (GLint*)hAlloca(utotal*sizeof(GLint));
    auto* uhashes = (hUint32*)hAlloca(utotal*sizeof(hUint32));

    for (auto i=0,n=utotal; i<n; ++i) {
        auto olen = 0;
        uary[i] = unames+(i*ulimit);
        glGetActiveUniform(rc->program_, i, ulimit, &olen, usizes+i, utypes+i, uary[i]);
        cyMurmurHash3_x86_32(uary[i], olen, hGetMurmurHashSeed(), uhashes+i);
    }

    /*
    totaluniforms = 0u;
    for (const auto& i : rcd.samplerStates_) {
        if (i.name_.is_default()) break;
        unames[totaluniforms++]=i.name_.c_str();
    }
    for (const auto& i : rcd.textureSlots_) {
        if (i.name_.is_default()) break;
        unames[totaluniforms++]=i.name_.c_str();
    }
    for (const auto& i : rcd.uniformBuffers_) {
        if (i.name_.is_default()) break;
        unames[totaluniforms++]=i.name_.c_str();
    }
    glGetUniformIndices(rc->program_, totaluniforms, unames, uindices);
    */

    //calculate the size of op codes required
    hGLRCHeader header = {0};
    //header

    return rc;
}

void swapBuffers() {
}
   
}
}
