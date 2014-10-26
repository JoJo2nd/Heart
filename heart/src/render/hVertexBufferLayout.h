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

    struct hVertexBufferLayout {
        hStringID        bindPoint_;
        hUint8           elementCount_;
        hVertexInputType type_;
        hUint16          offset_;
        hBool            normalised_;
        hUint16          stride_;
    };

}    
}