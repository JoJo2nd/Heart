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

struct StreamInfo
{
    Heart::hInputLayoutDesc desc;
    hUint                   count;
    const void*             encodedData;
    hUint                   encodedDataSize;
    void*                   decodedData;
    hUint                   decodedDataSize;
};

//////////////////////////////////////////////////////////////////////////
// Enum Tables ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Heart::hXMLEnumReamp g_formatTypes[] =
{
    {"FLOAT1",      Heart::eIF_FLOAT1       },
    {"FLOAT2",      Heart::eIF_FLOAT2       },
    {"FLOAT3",      Heart::eIF_FLOAT3       },
    {"FLOAT4",      Heart::eIF_FLOAT4       },
    {"UBTYE4UNORM", Heart::eIF_UBYTE4_UNORM },
    {"UBTYE4SNORM", Heart::eIF_UBYTE4_SNORM },
};

Heart::hXMLEnumReamp g_semanticTypes[] =
{
    {"POSITION",    Heart::eIS_POSITION  },
    {"NORMAL",      Heart::eIS_NORMAL    },
    {"TEXCOORD",    Heart::eIS_TEXCOORD  },
    {"COLOUR",      Heart::eIS_COLOUR    },
    {"TANGENT",     Heart::eIS_TANGENT   },
    {"BITANGENT",   Heart::eIS_BITANGENT },
    {"INSTANCE",    Heart::eIS_INSTANCE  },
};
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void WriteLODRenderables(const Heart::hXMLGetter& xLODData, LODHeader* header, Heart::hISerialiseStream* binoutput, Heart::hMemoryHeapBase* heap);
void GetMeshBounds(const hFloat* in, hUint inele, hUint verts, hFloat* min, hFloat* max);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT 
void HEART_API HeartGetBuilderVersion(hUint32* verMajor, hUint32* verMinor) {
    *verMajor = 0;
    *verMinor = 7;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int inputDescQsortCompar(const void* a, const void* b) {
    StreamInfo* lhs=(StreamInfo*)a;
    StreamInfo* rhs=(StreamInfo*)b;
    return ((0xFF-(lhs->desc.inputStream_-rhs->desc.inputStream_)) << 16) | (0xFF-(lhs->desc.semantic_-rhs->desc.semantic_));
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
            renderer->CreateIndexBuffer(tmpBuffer, rHeader.nPrimatives*3, 0, &ib);

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
hBool HEART_API HeartDataCompiler( Heart::hIDataCacheFile* inFile, Heart::hIBuiltDataCache* fileCache, 
Heart::hIDataParameterSet* params, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine, Heart::hISerialiseStream* binoutput )
{
    using namespace Heart;
    MeshHeader header = {0};
    hXMLDocument xmldoc;
    hChar* xmlmem = NULL;
    LODInfo* lodInfo = NULL;
    hUint32 lodIdx;
    hChar* pathroot = (hChar*)hAlloca(hStrLen(params->GetInputFilePath()));

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

    header.resHeader.resourceType = MESH_MAGIC_NUM;
    header.version = MESH_VERSION;
    header.lodCount=0;

    hXMLGetter xLODGetter = hXMLGetter(&xmldoc).FirstChild("modeldescription").FirstChild("lod");
    for (hUint32 i = 0;xLODGetter.ToNode(); xLODGetter = xLODGetter.NextSibling(), ++i) ++header.lodCount;
    lodInfo = (LODInfo*)hAlloca(sizeof(LODInfo)*header.lodCount);
    xLODGetter = hXMLGetter(&xmldoc).FirstChild("modeldescription").FirstChild("lod");
    for (hUint32 i = 0;xLODGetter.ToNode(); xLODGetter = xLODGetter.NextSibling(), ++i)
    {
        lodInfo[i].maxRange = xLODGetter.GetAttributeFloat("range",1000.f);
    }

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

        WriteLODRenderables(xLODData, &lodHeader, binoutput, memalloc->tempHeap_);

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

void WriteLODRenderables(const Heart::hXMLGetter& xLODData, LODHeader* header, Heart::hISerialiseStream* binoutput, Heart::hMemoryHeapBase* heap) {
    hcAssert(header && binoutput && heap);
    Heart::hXMLGetter renderablenode = xLODData.FirstChild("renderable");
    header->renderableCount = 0;
    header->renderableOffset = binoutput->Tell();

    for (; renderablenode.ToNode(); renderablenode=renderablenode.NextSibling()) {
        RenderableHeader renderableHeader = {0};
        hUint64 writeOffset = binoutput->Tell();
        Heart::hXMLGetter indexnode=renderablenode.FirstChild("index");
        void* indexData=NULL;
        hUint streamcount=0;
        for (Heart::hXMLGetter s=renderablenode.FirstChild("stream"); s.ToNode(); s=s.NextSibling()) ++streamcount;
        StreamInfo posStream;
        Heart::hXMLGetter* streamnodes=(Heart::hXMLGetter*)hAlloca(sizeof(Heart::hXMLGetter)*streamcount);
        StreamInfo* streaminfos=(StreamInfo*)hAlloca(sizeof(StreamInfo)*streamcount);
        for (Heart::hXMLGetter s=renderablenode.FirstChild("stream"); s.ToNode(); s=s.NextSibling(), ++streaminfos) {
            *streamnodes=s;
            streaminfos->encodedData=s.GetValueString();
            streaminfos->encodedDataSize=s.GetValueStringLen();
            streaminfos->decodedDataSize=Heart::hBase64::DecodeCalcRequiredSize(streaminfos->encodedData, streaminfos->encodedDataSize);
            streaminfos->decodedData=hHeapMalloc(heap, streaminfos->decodedDataSize);
            streaminfos->count=s.GetAttributeInt("count", 0);
            streaminfos->desc.inputStream_=s.GetAttributeInt("sindex", 0);
            streaminfos->desc.semIndex_=s.GetAttributeInt("index", 0);
            streaminfos->desc.semantic_=s.GetAttributeEnum("semantic", g_semanticTypes, Heart::eIS_POSITION);
            streaminfos->desc.typeFormat_=s.GetAttributeEnum("type", g_formatTypes, Heart::eIF_FLOAT1);
            streaminfos->desc.instanceDataRepeat_=0;
            Heart::hBase64::Decode(streaminfos->encodedData, streaminfos->encodedDataSize, 
                streaminfos->decodedData, streaminfos->decodedDataSize);
            if (streaminfos->desc.semantic_==Heart::eIS_POSITION) {
                posStream=*streaminfos;
            }
        }
        streaminfos-=streamcount;
        ++header->renderableCount;

        renderableHeader.flags=0;
        renderableHeader.primType = Heart::PRIMITIVETYPE_TRILIST;
        renderableHeader.startIndex = 0;
        renderableHeader.materialID = Heart::hResourceManager::BuildResourceID(renderablenode.GetAttributeString("material"));
        if (indexnode.ToNode()) {
            renderableHeader.nPrimatives = indexnode.GetAttributeInt("count",0);
            renderableHeader.nPrimatives /= 3;
            renderableHeader.flags|=indexnode.GetAttributeInt("count",0) > 0xFFFF ? MESH_DATA_FLAG_32BIT_INDEX : 0;
        } else {
            renderableHeader.nPrimatives = posStream.count/3;
        }

        // Update bounds
        GetMeshBounds((hFloat*)posStream.decodedData, (posStream.desc.typeFormat_-Heart::eIF_FLOAT1)+1, 
            posStream.count, renderableHeader.boundsMin, renderableHeader.boundsMax);
        for (hUint32 i = 0; i < 3; ++i)
        {
            header->boundsMin[i] = hMin(header->boundsMin[i], renderableHeader.boundsMin[i]);
            header->boundsMax[i] = hMax(header->boundsMax[i], renderableHeader.boundsMax[i]);
        }

        hUint vertexStreamCount=0;
        StreamInfo* inputDesc = (StreamInfo*)hAlloca(sizeof(StreamInfo)*streamcount);
        Heart::hMemCpy(inputDesc, streaminfos, sizeof(StreamInfo)*streamcount);
        qsort(inputDesc, streamcount, sizeof(StreamInfo), inputDescQsortCompar); //Sort by vertex stream index
        vertexStreamCount=inputDesc[streamcount-1].desc.inputStream_+1;
        binoutput->Write(&renderableHeader, sizeof(renderableHeader));
        for (hUint i=0; i<streamcount; ++i) {
            binoutput->Write(&inputDesc[i].desc, sizeof(Heart::hInputLayoutDesc));
        }
        renderableHeader.ibOffset = binoutput->Tell();
        if (indexnode.ToNode()) {
            hUint ibsize=Heart::hBase64::DecodeCalcRequiredSize(indexnode.GetValueString(), indexnode.GetValueStringLen());
            indexData=hHeapMalloc(heap, ibsize);
            Heart::hBase64::Decode(indexnode.GetValueString(), indexnode.GetValueStringLen(), indexData, ibsize);
            binoutput->Write(indexData, ibsize);
            hHeapFreeSafe(heap, indexData);
            renderableHeader.ibSize=ibsize;
            hcAssert(renderableHeader.nPrimatives*3*sizeof(hUint32) == ibsize || renderableHeader.nPrimatives*3*sizeof(hUint16) == ibsize);
        } else {
            renderableHeader.ibSize=0;
        }

        renderableHeader.verts = posStream.count;
        renderableHeader.inputElements = streamcount;
        renderableHeader.streams = vertexStreamCount;

        for (hUint streamIdx = 0; streamIdx < vertexStreamCount; ++streamIdx) {
            hUint64 streamOffset = binoutput->Tell();
            StreamHeader streamHeader = {streamIdx, 0};
            binoutput->Write(&streamHeader, sizeof(streamHeader));
            for (hUint vtxIdx = 0; vtxIdx < posStream.count; ++vtxIdx) {
                for (hUint32 inElem = 0; inElem < streamcount; ++inElem) {
                    if (inputDesc[inElem].desc.inputStream_ != streamIdx) {
                        continue;
                    }
                    hUint eleSize=inputDesc[inElem].decodedDataSize/inputDesc[inElem].count;
                    hUint offset=eleSize*vtxIdx;
                    binoutput->Write(((hByte*)inputDesc[inElem].decodedData)+offset, eleSize);
                    streamHeader.size+=eleSize;
                }
            }

            binoutput->Seek(streamOffset, Heart::hISerialiseStream::eBegin);
            binoutput->Write(&streamHeader, sizeof(streamHeader));
            binoutput->Seek(0, Heart::hISerialiseStream::eEnd);

            for (hUint ine=0,inen=streamcount; ine<inen; ++ine) {
                hHeapFreeSafe(heap, inputDesc[ine].decodedData);
            }
        }

        binoutput->Seek(writeOffset, Heart::hISerialiseStream::eBegin);
        binoutput->Write(&renderableHeader, sizeof(renderableHeader));
        binoutput->Seek(0, Heart::hISerialiseStream::eEnd);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void GetMeshBounds(const hFloat* in, hUint inele, hUint verts, hFloat* min, hFloat* max)
{
    for (hUint i = 0; i < 3; ++i)
    {
        min[i] = FLT_MAX;
        max[i] = -FLT_MAX;
    }
    for (hUint t=0,n=verts*inele; t < n; t+=inele)  {
        for (hUint i=0; i<inele; ++i) {
            min[i]=hMin(min[i],in[t+i]);
            max[i]=hMax(max[i],in[t+i]);
        }
        for (hUint i=inele; i<3; ++i) {
            min[i]=-.5f;max[i]=.5f;
        }
    }
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
            if (lod->renderObjects_[j].GetMaterial() == 0) {
                hMaterial* mat = static_cast<hMaterial*>(engine->GetResourceManager()->ltGetResource(lod->renderObjects_[j].GetMaterialResourceID()));
                // Possible the material won't have loaded just yet...
                if (!mat) return hFalse; 
                lod->renderObjects_[j].SetMaterial(mat->createMaterialInstance(0));
                lod->renderObjects_[j].bind();
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
    using namespace Heart;
    hRenderModel* rmodel = static_cast< hRenderModel* >(resource);
    for (hUint32 lIdx = 0, lodc = rmodel->GetLODCount(); lIdx < lodc; ++lIdx) {
        hGeomLODLevel* lod = rmodel->GetLOD(lIdx);
        for (hUint32 rIdx = 0, rCnt = lod->renderObjects_.GetSize(); rIdx < rCnt; ++rIdx) {
            hMaterialInstance::destroyMaterialInstance(lod->renderObjects_[rIdx].GetMaterial());
        }
    }
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
            for (hUint32 s = 0, sc=HEART_MAX_INPUT_STREAMS; s < sc; ++s) {
                hVertexBuffer* vb=lod->renderObjects_[rIdx].GetVertexBuffer(s);
                if (vb) {
                    renderer->DestroyVertexBuffer(vb);
                }
            }
            renderer->DestroyIndexBuffer(lod->renderObjects_[rIdx].GetIndexBuffer());
        }
    }

    hDELETE_SAFE(memalloc->resourcePakHeap_, resource);
}

