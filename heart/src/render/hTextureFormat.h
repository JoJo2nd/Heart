/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

namespace Heart {

    enum class hTextureFormat : hUint32 {
        Unknown,
/* !!JM TODO: Add formats as needed
        RGBA32_typeless,
        RGBA32_float,
        RGBA32_uint,
        RGBA32_sint,

        RGB32_typeless,  // DXGI_FORMAT_R32G32B32_TYPELESS          = 5,
        RGB32_float,     // DXGI_FORMAT_R32G32B32_FLOAT             = 6,
        RGB32_uint,      // DXGI_FORMAT_R32G32B32_UINT              = 7,
        RGB32_sint,      // DXGI_FORMAT_R32G32B32_SINT              = 8,
        
        RGBA16_typeless, // DXGI_FORMAT_R16G16B16A16_TYPELESS       = 9,
        RGBA16_float,    // DXGI_FORMAT_R16G16B16A16_FLOAT          = 10,
        RGBA16_unorm,    // DXGI_FORMAT_R16G16B16A16_UNORM          = 11,
        RGBA16_uint,     // DXGI_FORMAT_R16G16B16A16_UINT           = 12,
        RGBA16_snorm,    // DXGI_FORMAT_R16G16B16A16_SNORM          = 13,
        RGBA16_sint,     // DXGI_FORMAT_R16G16B16A16_SINT           = 14,
        
        RG32_typeless,    // DXGI_FORMAT_R32G32_TYPELESS             = 15,
        RG32_float,      // DXGI_FORMAT_R32G32_FLOAT                = 16,
        RG32_uint,       // DXGI_FORMAT_R32G32_UINT                 = 17,
        RG32_sint,       // DXGI_FORMAT_R32G32_SINT                 = 18,
        
        RG16_typeless,    // DXGI_FORMAT_R16G16_TYPELESS             = 15,
        RG16_float,      // DXGI_FORMAT_R16G16_FLOAT                = 16,
        RG16_uint,       // DXGI_FORMAT_R16G16_UINT                 = 17,
        RG16_sint,       // DXGI_FORMAT_R16G16_SINT                 = 18,

        R32_typeless,    //
        R32_float,      //
        R32_uint,       //
        R32_sint,       //

        R16_typeless,    //
        R16_float,      //
        R16_uint,       //
        R16_sint,       //

        RGB10A2_typeless,    // DXGI_FORMAT_R10G10B10A2_TYPELESS        = 23,
        RGB10A2_unorm,       // DXGI_FORMAT_R10G10B10A2_UNORM           = 24,
        RGB10A2_uint,        // DXGI_FORMAT_R10G10B10A2_UINT            = 25,
*/
        RGBA8_unorm,
        D32_float,
        D24S8_float,
        R8_unorm,

        BC1_unorm, // DXT1
        BC2_unorm, // DXT3
        BC3_unorm, // DXT5
        BC4_unorm, // Grapyscale only - does sRGB make sense for this format?
        BC5_unorm, // RB only

        sRGB_mask            = 0x80000000,
        RGBA8_sRGB_unorm     = RGBA8_unorm    | sRGB_mask,
        BC1_sRGB_unorm       = BC1_unorm      | sRGB_mask,
        BC2_sRGB_unorm       = BC2_unorm      | sRGB_mask,
        BC3_sRGB_unorm       = BC3_unorm      | sRGB_mask,
    };

}