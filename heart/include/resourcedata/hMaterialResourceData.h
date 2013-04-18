/********************************************************************

    filename:   hMaterialResourceData.h  
    
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

#ifndef HMATERIALRESOURCEDATA_H__
#define HMATERIALRESOURCEDATA_H__

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define MATERIAL_MAGIC_NUM              hMAKE_FOURCC('h','M','F','X')
#define MATERIAL_STRING_MAX_LEN         (32)
#define MATERIAL_PARAM_STRING_MAX_LEN   (64)
#define MATERIAL_DATA_MAX_SIZE          (16)
#define MATERIAL_COLOUR_SIZE            (4)
#define MATERIAL_MAJOR_VERSION          (((hUint16)1))
#define MATERIAL_MINOR_VERSION          (((hUint16)0))
#define MATERIAL_VERSION                ((MATERIAL_MAJOR_VERSION << 16)|MATERIAL_MINOR_VERSION)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)

struct MaterialHeader
{
    Heart::hResourceBinHeader   resHeader;
    hUint32                     version;
    hByte                       samplerCount;
    hUint16                     samplerRemapCount;
    hUint16                     parameterCount;
    hUint32                     parameterRemapCount;
    hByte                       groupCount;
    hUint32                     techniqueCount;
    hUint32                     passCount;
    hChar                       defaultGroupName[MATERIAL_STRING_MAX_LEN];
    hUint64                     samplerOffset;      //always after header
    hUint64                     parameterOffset;
    hUint64                     groupOffset;
};

struct SamplerDefinition
{
    hChar                       samplerName[MATERIAL_STRING_MAX_LEN];
    Heart::hResourceID          defaultTextureID;
    Heart::hSamplerStateDesc    samplerState;
    hUint8                      remapParamCount;
};

struct ParameterDefinition
{
    hChar                       parameterName[MATERIAL_PARAM_STRING_MAX_LEN];
    hUint                       count;
    Heart::hParameterType       type;
    union {
        hFloat                  floatData[MATERIAL_DATA_MAX_SIZE];
        hInt                    intData[MATERIAL_DATA_MAX_SIZE];
        hFloat                  colourData[MATERIAL_COLOUR_SIZE];
    };
};

struct GroupDefinition
{
    hChar               groupName[MATERIAL_STRING_MAX_LEN];
    hUint16             techniques;
};

struct TechniqueDefinition
{
    hChar               technqiueName[MATERIAL_STRING_MAX_LEN];
    hUint8              transparent;
    hUint8              layer;
    hUint16             passes;
};

struct PassDefintion
{
    hUint16                             pass;
    Heart::hBlendStateDesc              blendState;
    Heart::hDepthStencilStateDesc       depthState;
    Heart::hRasterizerStateDesc         rasterizerState;
    Heart::hResourceID                  vertexProgramID;
    Heart::hResourceID                  fragmentProgramID;
    Heart::hResourceID                  geometryProgramID;
    Heart::hResourceID                  hullProgramID;
    Heart::hResourceID                  domainProgramID;
};

#pragma pack(pop)

#endif // HMATERIALRESOURCEDATA_H__