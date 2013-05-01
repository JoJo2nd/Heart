/********************************************************************

    filename:   hMeshResourceData.h  
    
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

#ifndef HMESHRESOURCEDATA_H__
#define HMESHRESOURCEDATA_H__

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define MESH_MAGIC_NUM              hMAKE_FOURCC('h','M','S','H')
#define MESH_STRING_MAX_LEN         (32)
#define MESH_MAJOR_VERSION          (((hUint16)1))
#define MESH_MINOR_VERSION          (((hUint16)0))
#define MESH_VERSION                ((MESH_MAJOR_VERSION << 16)|MESH_MINOR_VERSION)

#pragma pack(push, 1)

#define MESH_DATA_FLAG_32BIT_INDEX (1)

struct MeshHeader
{
    Heart::hResourceBinHeader   resHeader;
    hUint32                     version;
    hUint32                     lodCount;
};

struct LODHeader
{
    hFloat          minRange;
    hFloat          maxRange;
    hFloat          boundsMin[3];
    hFloat          boundsMax[3];
    hUint32         renderableCount;
    hUint64         renderableOffset;
};

/*
    After the header is an array of hInputLayoutDesc[RenderableHeader.inputElements]
    then there is the index buffer, if it exists
    Then there is a StreamHeader followed by stream data
*/
struct RenderableHeader
{
    hUint32                 primType;
    hUint32                 startIndex;
    hUint32                 nPrimatives;
    hUint32                 verts;
    hFloat                  boundsMin[3];
    hFloat                  boundsMax[3];
    Heart::hResourceID      materialID;
    hUint32                 ibSize;
    hUint64                 ibOffset;   
    hUint32                 inputElements;
    hUint32                 streams; 
    hUint32                 flags;
};

struct StreamHeader
{
    hUint32 size;
    hUint16 index;
};

/*--- File Layout
+MeshHeader
+->LODHeader (x MeshHeader.lodCount)
+-->RenderableHeader (x LODHedaer.renderableCount)
+--->Heart::hInputLayoutDesc (x RenderableHeader.inputElements)
+--->hUint indexbuffer (x RenderableHeader.ibSize(bytes not ints)) (16 bit or 32 bit - check renderable header)
+--->StreamHeader (x RenderableHeader.streams)
---*/

#pragma pack(pop)

#endif // HMESHRESOURCEDATA_H__