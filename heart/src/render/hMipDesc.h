/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

namespace Heart {
namespace hRenderer {
    struct hMipDesc {
        hUint32      width;
        hUint32      height;
        const hByte* data;
        hUint32      size;
    };
}
}