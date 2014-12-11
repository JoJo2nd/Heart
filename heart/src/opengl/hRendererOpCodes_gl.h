/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include <GL/glew.h>
#include <GL/gl.h>

namespace Heart {
namespace hRenderer {

struct hUniformBuffer;
struct hTexture2D;
struct hRenderCall;
struct hRenderFence;

enum class Op : hUint8 {
    NoOp = 0,
    Clear,
    Swap,
	Draw,
    Jump,
	Fence,
	UniBufferFlush,
};

static hUint OpCodeSize = 1; // 1 byte

struct hGLJump {
    void* next;
};

struct hGLFence {
	hRenderFence* fence;
};

struct hGLRCHeader {
    union {
    struct {
        hBool   blend : 1;
        hBool   seperateAlpha : 1;
        hBool   depth : 1;
        hBool   stencil : 1;
        hBool   index : 1;
        hBool   writeMask : 1; // if true assume ~0u
        hBool   fill : 1;
        hBool   cullCW : 1;
        hBool   cullCCW : 1;
        hBool   scissor : 1;
        hBool   depthBais : 1;
    };
        hUint32 flags;
    };
    hUint8 samplerCount_;
    hUint8 uniBufferCount_;
    hUint8 textureCount_;
    hUint8 vtxAttCount_;

    hGLRCHeader() 
        : flags(0)
        , samplerCount_(0)
        , uniBufferCount_(0)
        , textureCount_(0)
        , vtxAttCount_(0) {

    }
};

struct hGLBlend {
    GLenum func;
    GLenum src;
    GLenum dest;
};

struct hGLDepth {
    GLenum  func;
    hUint32 mask;
};

struct hGLStencil {
    hUint32 readMask_;
    hUint32 writeMask_;
    hUint32 ref_;
    GLenum  failOp_;
    GLenum  depthFailOp_;
    GLenum  passOp_;
    GLenum  func_;
};

struct hGLDepthBais {
    hInt32  depthBias_;
    hFloat  depthBiasClamp_;
    hFloat  slopeScaledDepthBias_;
    hUint32 depthClipEnable_;
};

struct hGLVtxAttrib {
    GLuint  index_;
    GLint   size_;
    GLenum  type_;
    GLsizei stride_;
    void*   pointer_;
};

struct hGLSampler {
    GLint  index;
    GLuint samplerObj;
};

struct hGLTexture2D {
    GLint       index_;
    const hTexture2D* tex_;
};

struct hGLUniformBuffer {
    GLint           index_;
    const hUniformBuffer* ub_;
};

struct hGLClear {
    hColour colour;
    hFloat  depth;
};

struct hGLDraw {
	hRenderCall* rc;
	GLenum		 primType;
	hUint		 count;
};

struct hGLUniBufferFlush {
	hUniformBuffer* ub;
	hUint			offset;
	hUint			size;
};

}
}