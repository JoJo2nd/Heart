/********************************************************************

	filename: 	fontloader.cpp	
	
	Copyright (c) 29:7:2012 James Moran
	
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

#include "meshloader.h"
#include "MeshDataStructs.h"
#include "assimpfilewrappers.h"

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

// Removed, needs to be selectable
/*aiProcess_MakeLeftHanded |*/

#define MESH_AI_FLAGS (\
    aiProcess_CalcTangentSpace |\
    aiProcess_JoinIdenticalVertices |\
    aiProcess_Triangulate |\
    aiProcess_GenSmoothNormals |\
    aiProcess_SplitLargeMeshes |\
    aiProcess_RemoveRedundantMaterials |\
    aiProcess_OptimizeMeshes |\
    aiProcess_OptimizeGraph |\
    aiProcess_SortByPType)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct LODInfo
{
    hFloat maxRange;
};

struct MaterialMap
{
    const hChar*        first;
    const hChar*        second;
    Heart::hResourceID  resID;
};

//////////////////////////////////////////////////////////////////////////
// Enum Tables ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Heart::hXMLEnumReamp g_formatTypes[] =
{
    {"float",   Heart::eIF_FLOAT1},
    {"float2",  Heart::eIF_FLOAT2},
    {"float3",  Heart::eIF_FLOAT3},
    {"float4",  Heart::eIF_FLOAT4},
    {"ubyte4",  Heart::eIF_UBYTE4_UNORM},
    {"byte4",   Heart::eIF_UBYTE4_SNORM},
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void WriteLODRenderables(const Heart::hXMLGetter& xLODData, LODHeader* header, Heart::hISerialiseStream* binoutput, MaterialMap* materialMap, aiFileIO* aiFileIOctx);
hUint32 GetVertexBufferFormatElements(const Heart::hXMLGetter& xLODData);
hUint32 GetVertexBufferFormat(const Heart::hXMLGetter& xLODData, Heart::hInputLayoutDesc* outDesc, hUint32 maxOut, hUint32* streamCount);
Heart::hResourceID GetMaterialResourceIDFromMaterialIndex(const aiScene* scene, hUint32 i, MaterialMap* materialMap);
void GetMeshBounds(const aiMesh& mesh, hFloat* min, hFloat* max);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 WriteFloat3ToFloat(const aiVector3D& inv, Heart::hISerialiseStream* st)
{
    return st->Write(&inv.x, sizeof(hFloat));
}

hUint32 WriteFloat3ToFloat2(const aiVector3D& inv, Heart::hISerialiseStream* st)
{
    hUint32 w;
    w = st->Write(&inv.x, sizeof(hFloat));
    w += st->Write(&inv.y, sizeof(hFloat));
    return w;
}

hUint32 WriteFloat3ToFloat3(const aiVector3D& inv, Heart::hISerialiseStream* st)
{
    return st->Write(&inv, sizeof(aiVector3D));
}

hUint32 WriteFloat3ToFloat3SwapXZ(const aiVector3D& inv, Heart::hISerialiseStream* st)
{
    hUint32 w;
    w = st->Write(&inv.x, sizeof(hFloat));
    w += st->Write(&inv.z, sizeof(hFloat));
    w += st->Write(&inv.y, sizeof(hFloat));
    return w;
}

hUint32 WriteFloat3ToFloat4(const aiVector3D& inv, Heart::hISerialiseStream* st)
{
    static const hFloat cv = 1.f;
    hUint32 w = st->Write(&inv, sizeof(aiVector3D));
    w += st->Write(&cv, sizeof(hFloat));
    return w;
}

hUint32 WriteColourToFloat4(const aiColor4D& inv, Heart::hISerialiseStream* st)
{
    return st->Write(&inv, sizeof(aiColor4D));
}

hUint32 WriteColourToFloat(const aiColor4D& inv, Heart::hISerialiseStream* st)
{
    return st->Write(&inv, sizeof(hFloat));
}

hUint32 WriteColourToFloat2(const aiColor4D& inv, Heart::hISerialiseStream* st)
{
    return st->Write(&inv, sizeof(hFloat)*2);
}

hUint32 WriteColourToFloat3(const aiColor4D& inv, Heart::hISerialiseStream* st)
{
    return st->Write(&inv, sizeof(hFloat)*3);
}

hUint32 WriteColourToUbyte4(const aiColor4D& inv, Heart::hISerialiseStream* st)
{
    hByte c[] = {
        (hByte)((inv.r*255.f)+.5f),
        (hByte)((inv.g*255.f)+.5f),
        (hByte)((inv.b*255.f)+.5f),
        (hByte)((inv.a*255.f)+.5f),
    };
    return st->Write(c, sizeof(hByte)*4);
}

hUint32 WriteColourToByte4(const aiColor4D& inv, Heart::hISerialiseStream* st)
{
    hChar c[] = {
        (hByte)((inv.r*255.f)-126.5f),
        (hByte)((inv.g*255.f)-126.5f),
        (hByte)((inv.b*255.f)-126.5f),
        (hByte)((inv.a*255.f)-126.5f),
    };
    return st->Write(c, sizeof(hChar)*4);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
Heart::hResourceClassBase* HEART_API HeartBinLoader( Heart::hISerialiseStream* inStream, Heart::hIDataParameterSet*, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine )
{

    using namespace Heart;
    MeshHeader header = {0};
    hRenderer* renderer = engine->GetRenderer();
    hRenderModel* rmodel = hNEW(memalloc->resourcePakHeap_, hRenderModel)();
    hInputLayoutDesc inputDesc[32];
    hInputLayoutDesc streamInputDesc[32];
    void* tmpBuffer = NULL;
    hUint32 tmpbufsize = 0;

    inStream->Read(&header, sizeof(header));
    
    rmodel->SetLODCount(header.lodCount);

    for (hUint32 lIdx = 0; lIdx < header.lodCount; ++lIdx)
    {
        LODHeader lodHeader;
        hGeomLODLevel* lod = rmodel->GetLOD(lIdx);
        inStream->Read(&lodHeader, sizeof(lodHeader));

        lod->minRange_ = lodHeader.minRange;
        lod->renderObjects_.Resize(lodHeader.renderableCount);

        for (hUint32 rIdx = 0; rIdx < lodHeader.renderableCount; ++rIdx)
        {
            RenderableHeader rHeader;
            hRenderable* renderable = &lod->renderObjects_[rIdx];
            hIndexBuffer* ib;
            hVertexBuffer* vb;
            hVec3 bounds[2];//min,max;
            hAABB aabb;

            inStream->Read(&rHeader, sizeof(rHeader));
            hcAssert(rHeader.inputElements < 32);
            inStream->Read(inputDesc, sizeof(hInputLayoutDesc)*rHeader.inputElements);

            tmpbufsize = hMax(tmpbufsize, rHeader.ibSize);
            tmpBuffer = hHeapRealloc(memalloc->tempHeap_, tmpBuffer, tmpbufsize);
            
            bounds[0] = hVec3(rHeader.boundsMin[0], rHeader.boundsMin[1], rHeader.boundsMin[2]);
            bounds[1] = hVec3(rHeader.boundsMax[0], rHeader.boundsMax[1], rHeader.boundsMax[2]);
            aabb = hAABB::computeFromPointSet(bounds, 2);

            inStream->Read(tmpBuffer, rHeader.ibSize);
            renderer->CreateIndexBuffer(
                (hUint16*)tmpBuffer, 
                rHeader.nPrimatives*3, 
                0, 
                (PrimitiveType)rHeader.primType, 
                &ib );

            renderable->SetStartIndex(rHeader.startIndex);
            renderable->SetPrimativeCount(rHeader.nPrimatives);
            renderable->SetPrimativeType((PrimitiveType)rHeader.primType);
            renderable->SetAABB(aabb);
            renderable->SetIndexBuffer(ib);
            renderable->SetMaterialResourceID(rHeader.materialID);

            hUint32 streams = rHeader.streams;
            for (hUint32 streamIdx = 0; streamIdx < streams; ++streamIdx)
            {
                StreamHeader sHeader = {0};
                inStream->Read(&sHeader, sizeof(sHeader));

                tmpbufsize = hMax(tmpbufsize, sHeader.size);
                tmpBuffer = hHeapRealloc(memalloc->tempHeap_, tmpBuffer, tmpbufsize);

                inStream->Read(tmpBuffer, sHeader.size);

                //Builder stream inputDesc
                hUint32 sidc = rHeader.inputElements;
                hUint32 side = 0;
                for (hUint32 sid = 0; sid < sidc; ++sid)
                {
                    if (inputDesc[sid].inputStream_ == sHeader.index)
                    {
                        streamInputDesc[side] = inputDesc[sid];
                        ++side;
                    }
                }

                //TODO: fix
                renderer->CreateVertexBuffer(tmpBuffer, rHeader.verts, streamInputDesc, side, 0, memalloc->resourcePakHeap_, &vb);
                renderable->SetVertexBuffer(sHeader.index, vb);
            }
        }
    }

    hHeapFreeSafe(memalloc->tempHeap_, tmpBuffer);

    return rmodel;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
hBool HEART_API HeartDataCompiler( Heart::hIDataCacheFile* inFile, Heart::hIBuiltDataCache* fileCache, Heart::hIDataParameterSet* params, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine, Heart::hISerialiseStream* binoutput )
{
    using namespace Heart;
    MeshHeader header = {0};
    hXMLDocument xmldoc;
    hChar* xmlmem = NULL;
    LODInfo* lodInfo = NULL;
    hUint32 materialMapCount = 0;
    MaterialMap* materialMap = NULL;
    hUint32 lodIdx;
    hChar* pathroot = (hChar*)hAlloca(hStrLen(params->GetInputFilePath()));
    aiFileIO aiFileIOctx;

    hChar* end = hStrRChr(pathroot, '/');
    if (end == NULL) 
        pathroot[0] = 0;
    else 
        end = NULL;
    
    xmlmem = (hChar*)hHeapMalloc(memalloc->tempHeap_, inFile->Lenght()+1);
    inFile->Read(xmlmem, inFile->Lenght());
    xmlmem[inFile->Lenght()] = 0;

    if (xmldoc.ParseSafe< rapidxml::parse_default >(xmlmem, memalloc->tempHeap_) == hFalse)
        return hFalse;

    mlaiInitFileIO(&aiFileIOctx, memalloc, fileCache);

    header.resHeader.resourceType = MESH_MAGIC_NUM;
    header.version = MESH_VERSION;

    header.lodCount = hXMLGetter(&xmldoc).FirstChild("modeldescription").FirstChild("lodranges").GetAttributeInt("count",0);
    lodInfo = (LODInfo*)hAlloca(sizeof(LODInfo)*header.lodCount);
    hXMLGetter xLODGetter = hXMLGetter(&xmldoc).FirstChild("modeldescription").FirstChild("lodranges").FirstChild("range");
    for (hUint32 i = 0;xLODGetter.ToNode(); xLODGetter = xLODGetter.NextSibling(), ++i)
    {
        lodInfo[i].maxRange = xLODGetter.GetAttributeFloat("range",1000.f);
    }

    materialMapCount = hXMLGetter(&xmldoc).FirstChild("modeldescription").FirstChild("materialmap").GetAttributeInt("count", 0);
    materialMap = (MaterialMap*)hAlloca(sizeof(MaterialMap)*(materialMapCount+1));
    hXMLGetter xMaterialMapGetter = hXMLGetter(&xmldoc).FirstChild("modeldescription").FirstChild("materialmap").FirstChild("material");
    for (hUint32 i = 0; xMaterialMapGetter.ToNode(); xMaterialMapGetter = xMaterialMapGetter.NextSibling(), ++i)
    {
        materialMap[i].first = xMaterialMapGetter.GetAttributeString("from");
        materialMap[i].second = xMaterialMapGetter.GetAttributeString("to");
        materialMap[i].resID = Heart::hResourceManager::BuildResourceID(materialMap[i].second);
    }
    materialMap[materialMapCount].first  = NULL;
    materialMap[materialMapCount].second = NULL;
    materialMap[materialMapCount].resID  = hResourceID();

    binoutput->Write(&header, sizeof(header));

    lodIdx = 0;
    hXMLGetter xLODData = hXMLGetter(&xmldoc).FirstChild("modeldescription").FirstChild("lod");
    for (; xLODData.ToNode(); xLODData = xLODData.NextSibling(), ++lodIdx )
    {
        hUint64 writeOffset = binoutput->Tell();
        LODHeader lodHeader = {0};
        lodHeader.minRange = lodIdx == 0 ? 0.0f : lodInfo[lodIdx-1].maxRange;
        lodHeader.maxRange = lodInfo[lodIdx].maxRange;

        // Write a dummy header, WriteLODRenderables will fill the correct data
        // for the header and we'll write it again
        binoutput->Write(&lodHeader, sizeof(lodHeader));
        
        for (hUint32 i = 0; i < 3; ++i)
        {
            lodHeader.boundsMax[i] =  FLT_MAX;
            lodHeader.boundsMin[i] = -FLT_MAX;
        }

        WriteLODRenderables(xLODData, &lodHeader, binoutput, materialMap, &aiFileIOctx);

        // Write out with the correct data
        binoutput->Seek(writeOffset, hISerialiseStream::eBegin);
        binoutput->Write(&lodHeader, sizeof(lodHeader));

        binoutput->Seek(0, hISerialiseStream::eEnd);
    }

    mlaiDestroyFileIO(&aiFileIOctx);

    return hTrue;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void WriteLODRenderables(const Heart::hXMLGetter& xLODData, LODHeader* header, Heart::hISerialiseStream* binoutput, MaterialMap* materialMap, aiFileIO* aiFileIOctx)
{
    const aiScene* scene = NULL;

    hUint32 inputElements = GetVertexBufferFormatElements(xLODData);
    Heart::hInputLayoutDesc* inputDesc = (Heart::hInputLayoutDesc*)hAlloca(sizeof(Heart::hInputLayoutDesc)*inputElements);
    hUint32 inputStreams = 0;
    hUint32 vtxFormat = GetVertexBufferFormat(xLODData, inputDesc, inputElements, &inputStreams);    
    const hChar* sceneName = xLODData.FirstChild("collada").GetValueString();

    if (!sceneName)
        return;

    scene = aiImportFileEx(sceneName, MESH_AI_FLAGS, aiFileIOctx);

    header->renderableCount = scene->mNumMeshes;
    header->renderableOffset = binoutput->Tell();

    for (hUint32 meshIdx = 0; meshIdx < scene->mNumMeshes; ++meshIdx)
    {
        RenderableHeader renderableHeader = {0};
        hUint64 writeOffset = binoutput->Tell();
        aiMesh* mesh = scene->mMeshes[meshIdx];

        renderableHeader.primType = Heart::PRIMITIVETYPE_TRILIST;
        renderableHeader.nPrimatives = (hUint16)mesh->mNumFaces;
        renderableHeader.startIndex = 0;
        renderableHeader.materialID = GetMaterialResourceIDFromMaterialIndex(scene, mesh->mMaterialIndex, materialMap);

        // Update bounds
        GetMeshBounds(*mesh, renderableHeader.boundsMin, renderableHeader.boundsMax);
        for (hUint32 i = 0; i < 3; ++i)
        {
            header->boundsMin[i] = hMin(header->boundsMin[i], renderableHeader.boundsMin[i]);
            header->boundsMax[i] = hMax(header->boundsMax[i], renderableHeader.boundsMax[i]);
        }

        binoutput->Write(&renderableHeader, sizeof(renderableHeader));
        binoutput->Write(inputDesc, sizeof(Heart::hInputLayoutDesc)*inputElements);

        renderableHeader.ibOffset = binoutput->Tell();

        for (hUint32 faceIdx = 0; faceIdx < mesh->mNumFaces; ++faceIdx)
        {
            hUint16 idxs[] = 
            {
                (hUint16)mesh->mFaces[faceIdx].mIndices[0],
                (hUint16)mesh->mFaces[faceIdx].mIndices[1],
                (hUint16)mesh->mFaces[faceIdx].mIndices[2],
            };
            //Only handle triangles
            hcAssert(mesh->mFaces[faceIdx].mNumIndices == 3);
            renderableHeader.ibSize += binoutput->Write(idxs, sizeof(idxs));
        }

        renderableHeader.verts = mesh->mNumVertices;

        renderableHeader.inputElements = inputElements;
        renderableHeader.streams = inputStreams;

        for (hUint32 streamIdx = 0; streamIdx < inputStreams; ++streamIdx)
        {
            hUint64 streamOffset = binoutput->Tell();
            StreamHeader streamHeader = {streamIdx, 0};

            binoutput->Write(&streamHeader, sizeof(streamHeader));

            for (hUint32 vtxIdx = 0; vtxIdx < mesh->mNumVertices; ++vtxIdx)
            {
                for (hUint32 inElem = 0; inElem < inputElements; ++inElem)
                {
                    if (inputDesc[inElem].inputStream_ != streamIdx)
                        continue;
                    /*
                    * TODO: *Possibly* Pack transform onto vertices?
                    */
                    switch (inputDesc[inElem].semantic_)
                    {
                    case Heart::eIS_POSITION:
                        {
                            switch(inputDesc[inElem].typeFormat_)
                            {
                            case Heart::eIF_FLOAT1: streamHeader.size += WriteFloat3ToFloat(mesh->mVertices[vtxIdx], binoutput); break;
                            case Heart::eIF_FLOAT2: streamHeader.size += WriteFloat3ToFloat2(mesh->mVertices[vtxIdx], binoutput); break;
                            case Heart::eIF_FLOAT3: streamHeader.size += WriteFloat3ToFloat3SwapXZ(mesh->mVertices[vtxIdx], binoutput); break;
                            case Heart::eIF_FLOAT4: streamHeader.size += WriteFloat3ToFloat4(mesh->mVertices[vtxIdx], binoutput); break;
                            default: hcAssertFailMsg("Unsupported Format"); break;
                            }
                        }
                        break;
                    case Heart::eIS_NORMAL:
                        {
                            switch(inputDesc[inElem].typeFormat_)
                            {
                            case Heart::eIF_FLOAT1: streamHeader.size += WriteFloat3ToFloat(mesh->mNormals[vtxIdx], binoutput); break;
                            case Heart::eIF_FLOAT2: streamHeader.size += WriteFloat3ToFloat2(mesh->mNormals[vtxIdx], binoutput); break;
                            case Heart::eIF_FLOAT3: streamHeader.size += WriteFloat3ToFloat3SwapXZ(mesh->mNormals[vtxIdx], binoutput); break;
                            case Heart::eIF_FLOAT4: streamHeader.size += WriteFloat3ToFloat4(mesh->mNormals[vtxIdx], binoutput); break;
                            default: hcAssertFailMsg("Unsupported Format"); break;
                            }
                        }
                        break;
                    case Heart::eIS_TANGENT:
                        {
                            switch(inputDesc[inElem].typeFormat_)
                            {
                            case Heart::eIF_FLOAT1: streamHeader.size += WriteFloat3ToFloat(mesh->mTangents[vtxIdx], binoutput); break;
                            case Heart::eIF_FLOAT2: streamHeader.size += WriteFloat3ToFloat2(mesh->mTangents[vtxIdx], binoutput); break;
                            case Heart::eIF_FLOAT3: streamHeader.size += WriteFloat3ToFloat3SwapXZ(mesh->mTangents[vtxIdx], binoutput); break;
                            case Heart::eIF_FLOAT4: streamHeader.size += WriteFloat3ToFloat4(mesh->mTangents[vtxIdx], binoutput); break;
                            default: hcAssertFailMsg("Unsupported Format"); break;
                            }
                        }
                        break;
                    case Heart::eIS_BITANGENT:
                        {
                            switch(inputDesc[inElem].typeFormat_)
                            {
                            case Heart::eIF_FLOAT1: streamHeader.size += WriteFloat3ToFloat(mesh->mBitangents[vtxIdx], binoutput); break;
                            case Heart::eIF_FLOAT2: streamHeader.size += WriteFloat3ToFloat2(mesh->mBitangents[vtxIdx], binoutput); break;
                            case Heart::eIF_FLOAT3: streamHeader.size += WriteFloat3ToFloat3SwapXZ(mesh->mBitangents[vtxIdx], binoutput); break;
                            case Heart::eIF_FLOAT4: streamHeader.size += WriteFloat3ToFloat4(mesh->mBitangents[vtxIdx], binoutput); break;
                            default: hcAssertFailMsg("Unsupported Format"); break;
                            }
                        }
                        break;
                    case Heart::eIS_TEXCOORD:
                        {
                            hUint32 uvidx = inputDesc[inElem].semIndex_;
                            switch(inputDesc[inElem].typeFormat_)
                            {
                            case Heart::eIF_FLOAT1: streamHeader.size += WriteFloat3ToFloat(mesh->mTextureCoords[uvidx][vtxIdx], binoutput); break;
                            case Heart::eIF_FLOAT2: streamHeader.size += WriteFloat3ToFloat2(mesh->mTextureCoords[uvidx][vtxIdx], binoutput); break;
                            case Heart::eIF_FLOAT3: streamHeader.size += WriteFloat3ToFloat3(mesh->mTextureCoords[uvidx][vtxIdx], binoutput); break;
                            case Heart::eIF_FLOAT4: streamHeader.size += WriteFloat3ToFloat4(mesh->mTextureCoords[uvidx][vtxIdx], binoutput); break;
                            default: hcAssertFailMsg("Unsupported Format"); break;
                            }
                        }
                        break;
                    case Heart::eIS_COLOUR:
                        {
                            hUint32 colidx = inputDesc[inElem].semIndex_;
                            switch(inputDesc[inElem].typeFormat_)
                            {
                            case Heart::eIF_FLOAT1: streamHeader.size += WriteColourToFloat(mesh->mColors[colidx][vtxIdx], binoutput); break;
                            case Heart::eIF_FLOAT2: streamHeader.size += WriteColourToFloat2(mesh->mColors[colidx][vtxIdx], binoutput); break;
                            case Heart::eIF_FLOAT3: streamHeader.size += WriteColourToFloat3(mesh->mColors[colidx][vtxIdx], binoutput); break;
                            case Heart::eIF_FLOAT4: streamHeader.size += WriteColourToFloat4(mesh->mColors[colidx][vtxIdx], binoutput); break;
                            case Heart::eIF_UBYTE4_SNORM: streamHeader.size += WriteColourToUbyte4(mesh->mColors[colidx][vtxIdx], binoutput); break;
                            case Heart::eIF_UBYTE4_UNORM: streamHeader.size += WriteColourToByte4(mesh->mColors[colidx][vtxIdx], binoutput); break;
                            default: hcAssertFailMsg("Unsupported Format"); break;
                            }
                        }
                        break;
                    }
                }
            }

            binoutput->Seek(streamOffset, Heart::hISerialiseStream::eBegin);
            binoutput->Write(&streamHeader, sizeof(streamHeader));
            binoutput->Seek(0, Heart::hISerialiseStream::eEnd);
        }

        binoutput->Seek(writeOffset, Heart::hISerialiseStream::eBegin);
        binoutput->Write(&renderableHeader, sizeof(renderableHeader));
        binoutput->Seek(0, Heart::hISerialiseStream::eEnd);
    }

    aiReleaseImport(scene);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void GetMeshBounds(const aiMesh& mesh, hFloat* min, hFloat* max)
{
    for (hUint32 i = 0; i < 3; ++i)
    {
        min[i] = FLT_MAX;
        max[i] = -FLT_MAX;
    }
    for (hUint32 t = 0; t < mesh.mNumVertices; ++t) 
    {
        aiVector3D tmp = mesh.mVertices[t];

        min[0] = hMin(min[0],tmp.x);
        min[1] = hMin(min[1],tmp.y);
        min[2] = hMin(min[2],tmp.z);

        max[0] = hMax(max[0],tmp.x);
        max[1] = hMax(max[1],tmp.y);
        max[2] = hMax(max[2],tmp.z);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Heart::hResourceID GetMaterialResourceIDFromMaterialIndex(const aiScene* scene, hUint32 idx, MaterialMap* materialMap)
{
    aiMaterial* mat = scene->mMaterials[idx];
    aiString name;
    
    mat->Get(AI_MATKEY_NAME, name);
    for (hUint32 i = 0; materialMap[i].first != NULL; ++i)
    {
        if (Heart::hStrCmp(materialMap[i].first, name.data) == 0)
            return materialMap[i].resID;
    }

    return Heart::hResourceID();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 GetVertexBufferFormatElements( const Heart::hXMLGetter& xLODData )
{
    hUint32 ret = 0;
    for (Heart::hXMLGetter xLayout(xLODData.FirstChild("layout").FirstChild(NULL)); xLayout.ToNode(); xLayout = xLayout.NextSiblingAny(), ++ret);
    return ret;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 GetVertexBufferFormat(const Heart::hXMLGetter& xLODData, Heart::hInputLayoutDesc* outDesc, hUint32 maxOut, hUint32* streamCount)
{
    /*
        TODO: this doesn't handle instance data yet
    */
    Heart::hXMLGetter xLayout(xLODData.FirstChild("layout"));
    Heart::hXMLGetter ie(xLODData.FirstChild("layout"));
    hUint32 iElement = 0;
    for (ie = xLayout.FirstChild(NULL); ie.ToNode(); ie = ie.NextSiblingAny())
    {
        if (Heart::hStrCmp(ie.ToNode()->name(),"position") == 0)
        {
            outDesc[iElement].inputStream_ = ie.GetAttributeInt("stream", 0);
            outDesc[iElement].semantic_ = Heart::eIS_POSITION;
            outDesc[iElement].semIndex_ = (hByte)ie.GetAttributeInt("index", 0);
            outDesc[iElement].typeFormat_ = ie.GetAttributeEnum("type", g_formatTypes, Heart::eIF_FLOAT3);
            outDesc[iElement].instanceDataRepeat_ = 0;

            *streamCount = hMax(*streamCount, outDesc[iElement].inputStream_+1);
            ++iElement;
            if (iElement >= maxOut) return iElement;
        }
        if (Heart::hStrCmp(ie.ToNode()->name(),"normal") == 0)
        {
            outDesc[iElement].inputStream_ = ie.GetAttributeInt("stream", 0);
            outDesc[iElement].semantic_ = Heart::eIS_NORMAL;
            outDesc[iElement].semIndex_ = (hByte)ie.GetAttributeInt("index", 0);
            outDesc[iElement].typeFormat_ = ie.GetAttributeEnum("type", g_formatTypes, Heart::eIF_FLOAT3);
            outDesc[iElement].instanceDataRepeat_ = 0;

            *streamCount = hMax(*streamCount, outDesc[iElement].inputStream_+1);
            ++iElement;
            if (iElement >= maxOut) return iElement;
        }
        if (Heart::hStrCmp(ie.ToNode()->name(),"colour") == 0)
        {
            outDesc[iElement].inputStream_ = ie.GetAttributeInt("stream", 0);
            outDesc[iElement].semantic_ = Heart::eIS_COLOUR;
            outDesc[iElement].semIndex_ = (hByte)ie.GetAttributeInt("index", 0);
            outDesc[iElement].typeFormat_ = ie.GetAttributeEnum("type", g_formatTypes, Heart::eIF_UBYTE4_UNORM);
            outDesc[iElement].instanceDataRepeat_ = 0;

            *streamCount = hMax(*streamCount, outDesc[iElement].inputStream_+1);
            ++iElement;
            if (iElement >= maxOut) return iElement;
        }
        if (Heart::hStrCmp(ie.ToNode()->name(),"tangent") == 0)
        {
            outDesc[iElement].inputStream_ = ie.GetAttributeInt("stream", 0);
            outDesc[iElement].semantic_ = Heart::eIS_TANGENT;
            outDesc[iElement].semIndex_ = (hByte)ie.GetAttributeInt("index", 0);
            outDesc[iElement].typeFormat_ = ie.GetAttributeEnum("type", g_formatTypes, Heart::eIF_FLOAT3);
            outDesc[iElement].instanceDataRepeat_ = 0;

            *streamCount = hMax(*streamCount, outDesc[iElement].inputStream_+1);
            ++iElement;
            if (iElement >= maxOut) return iElement;
        }
        if (Heart::hStrCmp(ie.ToNode()->name(),"bitangent") == 0)
        {
            outDesc[iElement].inputStream_ = ie.GetAttributeInt("stream", 0);
            outDesc[iElement].semantic_ = Heart::eIS_BITANGENT;
            outDesc[iElement].semIndex_ = (hByte)ie.GetAttributeInt("index", 0);
            outDesc[iElement].typeFormat_ = ie.GetAttributeEnum("type", g_formatTypes, Heart::eIF_FLOAT3);
            outDesc[iElement].instanceDataRepeat_ = 0;

            *streamCount = hMax(*streamCount, outDesc[iElement].inputStream_+1);
            ++iElement;
            if (iElement >= maxOut) return iElement;
        }
        if (Heart::hStrCmp(ie.ToNode()->name(),"texcoord") == 0)
        {
            outDesc[iElement].inputStream_ = ie.GetAttributeInt("stream", 0);
            outDesc[iElement].semantic_ = Heart::eIS_TEXCOORD;
            outDesc[iElement].semIndex_ = (hByte)ie.GetAttributeInt("index", 0);
            outDesc[iElement].typeFormat_ = ie.GetAttributeEnum("type", g_formatTypes, Heart::eIF_FLOAT2);
            outDesc[iElement].instanceDataRepeat_ = 0;

            *streamCount = hMax(*streamCount, outDesc[iElement].inputStream_+1);
            ++iElement;
            if (iElement >= maxOut) return iElement;
        }
    }


    return iElement;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
hBool HEART_API HeartPackageLink( Heart::hResourceClassBase* resource, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine )
{
    using namespace Heart;

    hRenderModel* rmodel = static_cast< hRenderModel* >(resource);

    hUint32 lodcount = rmodel->GetLODCount();
    for(hUint32 i = 0; i < lodcount; ++i)
    {
        hGeomLODLevel* lod = rmodel->GetLOD(i);
        hUint32 objcount = lod->renderObjects_.GetSize();
        for (hUint32 j = 0; j < objcount; ++j)
        {
            if (lod->renderObjects_[j].GetMaterialKey() == 0)
            {
                hMaterial* mat = static_cast<hMaterial*>(engine->GetResourceManager()->ltGetResource(lod->renderObjects_[j].GetMaterialResourceID()));
                // Possible the material won't have loaded just yet...
                if (!mat) return hFalse; 
                lod->renderObjects_[j].SetMaterial(mat);
            }
        }
    }

    return hTrue;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
void HEART_API HeartPackageUnlink( Heart::hResourceClassBase* resource, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine )
{

}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
void HEART_API HeartPackageUnload( Heart::hResourceClassBase* resource, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine )
{
    using namespace Heart;
    hRenderModel* rmodel = static_cast< hRenderModel* >(resource);
    hRenderer* renderer = engine->GetRenderer();

    for (hUint32 lIdx = 0, lodc = rmodel->GetLODCount(); lIdx < lodc; ++lIdx) {
        hGeomLODLevel* lod = rmodel->GetLOD(lIdx);
        for (hUint32 rIdx = 0, rCnt = lod->renderObjects_.GetSize(); rIdx < rCnt; ++rIdx) {
            renderer->DestroyIndexBuffer(lod->renderObjects_[rIdx].GetIndexBuffer());
            for (hUint32 s = 0, sc = lod->renderObjects_[rIdx].GetVertexStreams(); s < sc; ++s) {
                renderer->DestroyVertexBuffer(lod->renderObjects_[rIdx].GetVertexBuffer(s));
            }
        }
    }

    hDELETE_SAFE(memalloc->resourcePakHeap_, resource);
}

