/********************************************************************

    filename:   hShaderResourceData.h  
    
    Copyright (c) 18:4:2013 James Moran
    
    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.
    
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
    
    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
    
    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
    
    3. This notice may not be removed or altered from any source
    distribution.

*********************************************************************/
#pragma once

#ifndef HSHADERRESOURCEDATA_H__
#define HSHADERRESOURCEDATA_H__

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define SHADER_MAGIC_NUM              hMAKE_FOURCC('h','G','P','U')
#define SHADER_STRING_MAX_LEN         (32)
#define SHADER_MAJOR_VERSION          (((hUint16)1))
#define SHADER_MINOR_VERSION          (((hUint16)0))
#define SHADER_VERSION                ((SHADER_MAJOR_VERSION << 16)|SHADER_MINOR_VERSION)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)

struct ShaderHeader
{
    Heart::hResourceBinHeader   resHeader;
    hUint32                     version;
    Heart::hShaderType           type;
    hUint32                     vertexLayout;
    hUint32                     shaderBlobSize;
    hUint32                     inputLayoutElements;
};

#pragma pack(pop)

#endif // HSHADERRESOURCEDATA_H__