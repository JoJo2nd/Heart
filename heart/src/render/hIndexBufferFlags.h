/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

namespace Heart {
namespace hRenderer {
    enum class hIndexBufferFlags : hUint32 {
        DynamicBuffer       = 0x01,
        DwordIndices        = 0x02,
    };
}
}