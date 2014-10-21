/********************************************************************
    Written by James Moran
    Please see the file HEARTLICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

namespace Heart {
namespace hRenderer {
    
    enum class hColourWriteFlag : hUint32 {
        Red    = 1,
        Green  = 1 << 1,
        Blue   = 1 << 2,
        Alpha  = 1 << 3,
        Full   = 0x0000000F,
    };
} 
}