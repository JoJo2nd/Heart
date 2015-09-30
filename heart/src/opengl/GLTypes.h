/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "base/hMemory.h"
#include "opengl/hRendererOpCodes_gl.h"
#include "opengl/GLFuncs.h"
#include <GL/gl.h>
#include <memory>
#include <vector>
#include <functional>

struct lfds_freelist_element;

namespace Heart {
namespace hRenderer {
    struct hUniformLayoutDesc;

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
                hcPrintf("%s(%d): OpenGL Error %s", file, line, errorstr);
            }
        }
    };

#if HEART_DEBUG_INFO
#   define hGLErrorScope() Heart::hRenderer::hGLErrorSentry gl_error_sentry__blah_blah__(__FILE__, __LINE__)
#else
#   define hGLErrorScope()
#endif

#ifdef HEART_PLAT_LINUX
    typedef void* hGLDebugCallback_t;
#elif defined HEART_PLAT_WINDOWS
    typedef const void* hGLDebugCallback_t;
#endif

    enum class hParamCall {
        none,
        a1f,
        a2f,
        a3f,
        a4f,
        a1i,
        a2i,
        a3i,
        a4i,
        a1ui,
        a2ui,
        a3ui,
        a4ui,
        m2x2f,
        m2x3f,
        m2x4f,
        m3x2f,
        m3x3f,
        m3x4f,
        m4x2f,
        m4x3f,
        m4x4f,
    };

    struct hRenderFence {
        hRenderFence() 
            : element(nullptr)
            , sync(nullptr) {
        }
        lfds_freelist_element* element;
        GLsync     sync;
    };

    struct hGLObjectBase {
        hGLObjectBase()
            : persistantMapping(nullptr)
            , flags(0) {

        }

        enum class Flag : hUint8 {
            RenderThreadBound = 0x80, // Set on successful 
            ValidName = 0x40,
        };

        void*   persistantMapping;
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
        hUint32 alignedSize;
    };

    struct hUniformBuffer : hGLObjectBase {
        hUint   size_;
        hUint32 createFlags_;
        hUint	mappedOffset_;
        hUint	mappedSize_;
        hUint   layoutDescCount;
        hUniformLayoutDesc* layoutDesc;
        char*   fieldNames; // <- same allocation as layoutDesc, don't delete.
    };

    struct hTextureBase : hGLObjectBase {
        GLenum  target_;
        GLuint  internalFormat_;
        GLuint  format_;
    };

    struct hTexture2D : hTextureBase {

    };

    struct hShaderStage {
        GLuint  shaderObj_;
    };

    struct hGLSampler {
        GLint  index;
        //GLuint samplerObj;
    };

    struct hUniformParamUpdate {
        hParamCall type;
        hInt location;
        hUint16 uniformBufferOffset;
        hUint8 uniformBufferIndex;
        hUint8 paramSize;
    };

    struct hRenderCall {
        enum Flag : hUint8 {
            VAOBound = 0x80,
        };

        hRenderCall()
            : size_(0)
            //, opCodes_(nullptr, hFree)
            , paramUpdateCount(0)
            , flags(0) {
        }


        hRenderCall(const hRenderCall& rhs) = delete;
        hRenderCall& operator = (const hRenderCall& rhs) = delete;

        hGLRCHeader header_;
        GLuint program_;
        GLuint vao;
        hVertexBuffer* vtxBuf_;
        hUint size_;
        std::unique_ptr<hByte, std::function<void(hByte*)>> opCodes_;
        hGLSampler* samplers_;
        hUint paramUpdateCount;
        std::unique_ptr<hUniformParamUpdate[]> paramUpdates;
		std::vector<const hUniformBuffer*> uniformBuffers;
        hUint8  flags;
    };

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
                    cmdsSize += sizeof(hGLJump) + OpCodeSize;
                    *((Op*)j) = Op::Jump;
                    *((hUint32*)(j+4)) = 0;
                    ((hGLJump*)(j + OpCodeSize))->next = memnext;
                }
                nextcmd = memnext;
                // note remaining space but leave enough for another jump cmd
                cmdsReserve += MinCmdBlockSize - (sizeof(hGLJump) + OpCodeSize);
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
}
}