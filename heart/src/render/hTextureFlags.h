/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

namespace Heart {
namespace hRenderer {
    enum class TextureFlags : hUint32 {
        RenderTarget = 0x80000000,
        Dynamic = 0x80000001,
    };
}}
