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

enum class Op : hUint8 {
    NoOp = 0,
    SetRC,
    SetVtxAttrib,
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
};

struct hGLBlend {
    GLenum func_;
    GLenum src_;
    GLenum dest_;
};

struct hGLDepth {
    GLenum  func_;
    hUint32 mask_;
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
    hUint32 scissorEnable_;
};

}
}