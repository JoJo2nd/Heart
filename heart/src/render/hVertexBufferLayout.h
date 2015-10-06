/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "base/hTypes.h"
#include "base/hStringID.h"

namespace Heart {
namespace hRenderer {

    enum class hVertexInputType {
        Unknown = 0,
        Byte,
        UByte,
        Short,
        UShort,
        Int,
        UInt,
        HalfFloat,
        Float,
        Double,
    };

    enum class hSemantic {
        Position      = 0,
        Blendweight   = 1,
        Blendindices  = 2,
        Normal        = 3,
        Psize         = 4,
        Texcoord      = 5,
        Tangent       = 6,
        Binormal      = 7,
        Tessfactor    = 8,
        Positiont     = 9,
        Colour        = 10,
        Fog           = 11,
        Depth         = 12,
        Sample        = 13
    };

    struct hVertexBufferLayout {
        hStringID        bindPoint_;
        hSemantic        semantic_;
        hUint16          semIndex_;
        hUint8           elementCount_;
        hVertexInputType type_;
        hUint16          offset_;
        hBool            normalised_;
        hUint16          stride_;
    };

}    
}
