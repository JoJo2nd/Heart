/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "base/hTypes.h"
#include "base/hArray.h"
#include "base/hMemoryUtil.h"
#include <vector>

namespace Heart {
    struct hRect 
    {
        hUint32 top_,left_,right_,bottom_;

        hBool operator == (const hRect& rhs)
        {
            return left_ == rhs.left_ && right_ == rhs.right_ && top_ == rhs.top_ && bottom_ == rhs.bottom_;
        }

        hBool operator != (const hRect& rhs)
        {
            return !(*this == rhs);
        }
    };

    struct hViewport
    {
        hViewport() {}
        hViewport( hUint32 x,hUint32 y,hUint32 w,hUint32 h )
            : x_(x), y_(y), width_(w), height_(h)
        {}
        hViewport(hViewport&& rhs) {
            hPLACEMENT_NEW(this) hViewport();
            swap(*this, rhs);
        }
        const hViewport& operator = (hViewport rhs) {
            swap(*this, rhs);
            return *this;
        }
        void swap(hViewport& lhs, hViewport& rhs) {
            std::swap(lhs.x_, rhs.x_);
            std::swap(lhs.y_, rhs.y_);
            std::swap(lhs.width_, rhs.width_);
            std::swap(lhs.height_, rhs.height_);
        }
        hUint32 x_,y_,width_,height_;
    };
    struct hRelativeViewport 
    {
        hRelativeViewport() {}
        hRelativeViewport(hFloat fx, hFloat fy, hFloat fw, hFloat fh)
            : x(fx), y(fy), w(fw), h(fh)
        {}
        hFloat x, y, w, h;
    };

    typedef hRect hScissorRect;

    struct hRelativeScissorRect
    {
        hFloat left,top,right,bottom;

        hBool operator == (const hRelativeScissorRect& rhs)
        {
            return left == rhs.left && right == rhs.right && top == rhs.top && bottom == rhs.bottom;
        }

        hBool operator != (const hRelativeScissorRect& rhs)
        {
            return !(*this == rhs);
        }
    };

    struct hRenderFrameStats
    {
        hFloat          frametime_;
        hFloat          gpuTime_;
        hUint32			nPrims_;
        hUint32			nDeviceCalls_;
        hUint32			nDrawCalls_;
    };

    enum hShaderType
    {
        ShaderType_VERTEXPROG,
        ShaderType_FRAGMENTPROG,
        ShaderType_GEOMETRYPROG,
        ShaderType_HULLPROG,
        ShaderType_DOMAINPROG,

        ShaderType_COMPUTEPROG,

        ShaderType_MAX,
        ShaderType_FORCE_DWORD = ~0U
    };

    enum class hShaderProfile {
        ES2_vs, ES2_ps,
        WebGL_vs, WebGL_ps,
        ES3_vs, ES3_ps,
        D3D_9c_vs, D3D_9c_ps,
        FL10_vs, FL10_ps, FL10_gs, FL10_cs,
        FL11_vs, FL11_ps, FL11_gs, FL11_cs, FL11_hs, FL11_ds,

        // Nothing should support invalid
        Invalid,
        eShaderProfile_Max,
    };

    enum class hShaderFrequency {
        Vertex, Pixel, Hull, Domain, Compute
    };
/*
    enum hIndexBufferType {
        hIndexBufferType_Index16,
        hIndexBufferType_Index32,

        hIndexBufferType_Max
    };
*/
}//Heart

#include "base/hColour.h"
#include "render/hRenderStates.h"
