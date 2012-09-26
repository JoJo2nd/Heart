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

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#define MESH_AI_FLAGS (\
    aiProcess_CalcTangentSpace |\
    aiProcess_MakeLeftHanded |\
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
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void WriteLODRenderables(const Heart::hXMLGetter& xLODData, LODHeader* header, Heart::hISerialiseStream* binoutput, MaterialMap* materialMap);
hUint32 GetVertexBufferFormat(const Heart::hXMLGetter& xLODData);
Heart::hResourceID GetMaterialResourceIDFromMaterialIndex(const aiScene* scene, hUint32 i, MaterialMap* materialMap);
void GetMeshBounds(const aiMesh& mesh, hFloat* min, hFloat* max);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
Heart::hResourceClassBase* HEART_API HeartBinLoader( Heart::hISerialiseStream* inStream, Heart::hIDataParameterSet*, Heart::HeartEngine* engine )
{
    using namespace Heart;
    MeshHeader header = {0};
    hRenderer* renderer = engine->GetRenderer();
    hRenderModel* rmodel = hNEW(GetGlobalHeap(), hRenderModel)();
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
            tmpbufsize = hMax(tmpbufsize, rHeader.ibSize);
            tmpbufsize = hMax(tmpbufsize, rHeader.vbSize);
            tmpBuffer = hHeapRealloc(GetGlobalHeap(), tmpBuffer, tmpbufsize);
            
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

            inStream->Read(tmpBuffer, rHeader.vbSize);
            renderer->CreateVertexBuffer(tmpBuffer, rHeader.verts, rHeader.vbLayout, 0, &vb);

            renderable->SetStartIndex(rHeader.startIndex);
            renderable->SetPrimativeCount(rHeader.nPrimatives);
            renderable->SetPrimativeType((PrimitiveType)rHeader.primType);
            renderable->SetAABB(aabb);
            renderable->SetIndexBuffer(ib);
            renderable->SetVertexBuffer(vb);
        }
    }

    hHeapFreeSafe(GetGlobalHeap(), tmpBuffer);

    return rmodel;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
hBool HEART_API HeartDataCompiler( Heart::hIDataCacheFile* inFile, Heart::hIBuiltDataCache* fileCache, Heart::hIDataParameterSet* params, Heart::HeartEngine* engine, Heart::hISerialiseStream* binoutput )
{
    using namespace Heart;
    MeshHeader header = {0};
    hXMLDocument xmldoc;
    hChar* xmlmem = NULL;
    LODInfo* lodInfo = NULL;
    hUint32 materialMapCount = 0;
    MaterialMap* materialMap = NULL;
    hUint32 lodIdx;
    
    xmlmem = (hChar*)hHeapMalloc(GetGlobalHeap(), inFile->Lenght()+1);
    inFile->Read(xmlmem, inFile->Lenght());
    xmlmem[inFile->Lenght()] = 0;

    if (xmldoc.ParseSafe< rapidxml::parse_default >(xmlmem, GetGlobalHeap()) == hFalse)
        return hFalse;

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
    materialMap[materialMapCount].resID  = NULL;

    binoutput->Write(&header, sizeof(header));


    lodIdx = 0;
    hXMLGetter xLODData = hXMLGetter(&xmldoc).FirstChild("modeldescription").FirstChild("lod");
    for (; xLODData.ToNode(); xLODData = xLODData.NextSibling(), ++lodIdx )
    {
        hUint32 writeOffset = binoutput->Tell();
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

        WriteLODRenderables(xLODData, &lodHeader, binoutput, materialMap);

        // Write out with the correct data
        binoutput->Seek(writeOffset, hISerialiseStream::eBegin);
        binoutput->Write(&lodHeader, sizeof(lodHeader));

        binoutput->Seek(0, hISerialiseStream::eEnd);
    }

    return hTrue;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void WriteLODRenderables(const Heart::hXMLGetter& xLODData, LODHeader* header, Heart::hISerialiseStream* binoutput, MaterialMap* materialMap)
{
    const aiScene* scene = NULL;

    hUint32 vtxFormat = GetVertexBufferFormat(xLODData);
    const hChar* sceneName = xLODData.FirstChild("collada").GetValueString();

    if (!sceneName)
        return;

    scene = aiImportFile(sceneName, MESH_AI_FLAGS);

    header->renderableCount = scene->mNumMeshes;
    header->renderableOffset = binoutput->Tell();

    for (hUint32 meshIdx = 0; meshIdx < scene->mNumMeshes; ++meshIdx)
    {
        RenderableHeader renderableHeader = {0};
        hUint32 writeOffset = binoutput->Tell();
        aiMesh* mesh = scene->mMeshes[meshIdx];

        renderableHeader.vbLayout = vtxFormat;
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

        renderableHeader.vbOffset = binoutput->Tell();
        renderableHeader.verts = mesh->mNumVertices;
        for (hUint32 vtxIdx = 0; vtxIdx < mesh->mNumVertices; ++vtxIdx)
        {
            /*
                Vertex Buffer layout:- Make sure code generating vertex bufffers follow this
                Position,
                Normal,
                Tangent,
                Binormal,
                Colour,
                UV(1-8)
            */
            /*
             * TODO: *Possibly* Pack transform onto vertices?
             */
#define WRITE_UV_CHANNEL(uvx) \
            if (vtxFormat & Heart::hrVF_##uvx##UV)\
            {\
                renderableHeader.vbSize += binoutput->Write(&mesh->mTextureCoords[(uvx-1)][vtxIdx], sizeof(hFloat)*2);\
            }

            if (vtxFormat & Heart::hrVF_XYZ)
            {
                renderableHeader.vbSize += binoutput->Write(&mesh->mVertices[vtxIdx], sizeof(aiVector3D));
            }
            if (vtxFormat & Heart::hrVF_NORMAL)
            {
                renderableHeader.vbSize += binoutput->Write(&mesh->mNormals[vtxIdx], sizeof(aiVector3D));
            }
            if (vtxFormat & Heart::hrVF_TANGENT)
            {
                renderableHeader.vbSize += binoutput->Write(&mesh->mTangents[vtxIdx], sizeof(aiVector3D));
            }
            if (vtxFormat & Heart::hrVF_BINORMAL)
            {
                renderableHeader.vbSize += binoutput->Write(&mesh->mBitangents[vtxIdx], sizeof(aiVector3D));
            }
            if (vtxFormat & Heart::hrVF_COLOR)
            {
                renderableHeader.vbSize += binoutput->Write(&mesh->mColors[0][vtxIdx], sizeof(aiColor4D));
            }
            WRITE_UV_CHANNEL(1);
            WRITE_UV_CHANNEL(2);
            WRITE_UV_CHANNEL(3);
            WRITE_UV_CHANNEL(4);
            WRITE_UV_CHANNEL(5);
            WRITE_UV_CHANNEL(6);
            WRITE_UV_CHANNEL(7);
            WRITE_UV_CHANNEL(8);

#undef WRITE_UV_CHANNEL
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

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 GetVertexBufferFormat(const Heart::hXMLGetter& xLODData)
{
    Heart::hXMLGetter xLayout(xLODData.FirstChild("layout"));
    hUint32 vtxFmt = 0;
    if (xLayout.FirstChild("position").ToNode())    vtxFmt |= Heart::hrVF_XYZ;
    if (xLayout.FirstChild("normal").ToNode())      vtxFmt |= Heart::hrVF_NORMAL;
    if (xLayout.FirstChild("colour").ToNode())      vtxFmt |= Heart::hrVF_COLOR;
    if (xLayout.FirstChild("tangent").ToNode())     vtxFmt |= Heart::hrVF_TANGENT;
    if (xLayout.FirstChild("binormal").ToNode())    vtxFmt |= Heart::hrVF_BINORMAL;
    if (xLayout.FirstChild("uv1").ToNode())         vtxFmt |= Heart::hrVF_1UV;
    if (xLayout.FirstChild("uv2").ToNode())         vtxFmt |= Heart::hrVF_2UV;
    if (xLayout.FirstChild("uv3").ToNode())         vtxFmt |= Heart::hrVF_3UV;
    if (xLayout.FirstChild("uv4").ToNode())         vtxFmt |= Heart::hrVF_4UV;
    if (xLayout.FirstChild("uv5").ToNode())         vtxFmt |= Heart::hrVF_5UV;
    if (xLayout.FirstChild("uv6").ToNode())         vtxFmt |= Heart::hrVF_6UV;
    if (xLayout.FirstChild("uv7").ToNode())         vtxFmt |= Heart::hrVF_7UV;
    if (xLayout.FirstChild("uv8").ToNode())         vtxFmt |= Heart::hrVF_8UV;

    return vtxFmt;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
hBool HEART_API HeartPackageLink( Heart::hResourceClassBase* resource, Heart::HeartEngine* engine )
{
    using namespace Heart;

    return hTrue;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
void HEART_API HeartPackageUnlink( Heart::hResourceClassBase* resource, Heart::HeartEngine* engine )
{

}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
void HEART_API HeartPackageUnload( Heart::hResourceClassBase* resource, Heart::HeartEngine* engine )
{

}

