/********************************************************************

    filename:   mesh_lod_level.cpp  
    
    Copyright (c) 19:1:2013 James Moran
    
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

#include "precompiled.h"
#include "mesh/mesh_lod_level.h"
#include "boost/filesystem.hpp"
#include <sstream>

#define MESH_AI_FLAGS (\
    aiProcess_CalcTangentSpace |\
    aiProcess_JoinIdenticalVertices |\
    aiProcess_Triangulate |\
    aiProcess_ImproveCacheLocality |\
    aiProcess_GenSmoothNormals |\
    aiProcess_SplitLargeMeshes |\
    aiProcess_FindInvalidData |\
    aiProcess_ConvertToLeftHanded |\
    aiProcess_RemoveRedundantMaterials |\
    aiProcess_OptimizeMeshes |\
    aiProcess_OptimizeGraph |\
    aiProcess_SortByPType | \
    aiProcess_TransformUVCoords)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#define AI_COlOUR_TO_UBYTE(c) \
    (long)((((unsigned char)(c[0]*256.f))<<24)|(((unsigned char)(c[1]*256.f))<<16)|(((unsigned char)(c[2]*256.f))<<8)|(((unsigned char)(c[3]*256.f))))

#define AI_COlOUR_TO_SBYTE(c) \
    (long)((((unsigned char)(c[0]*512.f-256.f))<<24)|(((unsigned char)(c[1]*512.f-256.f))<<16)|(((unsigned char)(c[2]*512.f-256.f))<<8)|(((unsigned char)(c[3]*512.f-256.f))))

#define MDF_EXPORT_CHECK1(x, str1) \
    if (!(x)) {\
        ret.exportOK=false;\
        ret.errors=str1;\
        return ret;\
    }

#define MDF_EXPORT_CHECK2(x, str1, str2) \
    if (!(x)) {\
        ret.exportOK=false;\
        ret.errors=str1;\
        ret.errors+=str2;\
        return ret;\
    }

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
const char* getSemanticName(Heart::hInputSemantic sem) {
    switch(sem) {
    case Heart::eIS_POSITION:   return "POSITION";
    case Heart::eIS_NORMAL:     return "NORMAL";
    case Heart::eIS_TEXCOORD:   return "TEXCOORD";
    case Heart::eIS_COLOUR:     return "COLOUR";
    case Heart::eIS_TANGENT:    return "TANGENT";
    case Heart::eIS_BITANGENT:  return "BITANGENT";
    case Heart::eIS_INSTANCE:   return "INSTANCE";
    }
    return "UNKNOWN";
}

const char* getFormatName(Heart::hInputFormat sem) {
    switch(sem) {
    case Heart::eIF_FLOAT4:         return "FLOAT4";
    case Heart::eIF_FLOAT3:         return "FLOAT3";
    case Heart::eIF_FLOAT2:         return "FLOAT2";
    case Heart::eIF_FLOAT1:         return "FLOAT1";
    case Heart::eIF_UBYTE4_UNORM:   return "UBTYE4UNORM";
    case Heart::eIF_UBYTE4_SNORM:   return "UBTYE4SNORM";
    }
    return "UNKNOWN";
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template< typename t_vec, size_t indim, size_t outdim >
void writeVecToFloat(const t_vec& v, float* out) {
    for (size_t i=0; i<outdim; ++i) {
        if (i<indim) {
            out[i]=v[i];
        } else {
            out[i]= i >= 4 ? 1.f : 0.f;
        }
    }
}

template< typename t_vec, size_t indim, size_t outdim >
void writeUVVecToFloat(const t_vec& v, float* out) {
    for (size_t i=0; i<outdim; ++i) {
        if (i<indim) {
            out[i]= v[i];
        } else {
            out[i]= i >= 4 ? 1.f : 0.f;
        }
    }
    if (outdim >= 2) {
        out[1]=1.f-out[1];
    }
}

template< typename t_vec, size_t indim, size_t outdim >
void writeVecToFloatSwap(const t_vec& v, float* out) {
    for (size_t i=0; i<outdim; ++i) {
        if (i<indim) {
            out[i]=v[i];
        } else {
            out[i]= i >= 4 ? 1.f : 0.f;
        }
    }
    if (outdim>2) {
        //swap Z<->Y
        float t=out[1];
        out[1]=out[2];
        out[2]=t;
    }
}

template< typename t_ty >
void writeVecToByteUnorm(const t_ty& v, long* out) {
    out[0]=AI_COlOUR_TO_UBYTE(v);
}

template< typename t_ty >
void writeVecToByteSnorm(const t_ty& v, long* out) {
    out[0]=AI_COlOUR_TO_SBYTE(v);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template< typename t_ty, size_t t_indim, bool t_isUV >
rapidxml::xml_node<>* writeStreamNode(xml_doc* doc, Heart::hInputSemantic semantic, Heart::hInputFormat typeformat, size_t semIdx, 
size_t elements, const t_ty* inputbuf, bool axisswap=false) {
    using namespace rapidxml;

    boost::shared_array<char> convertedBuffer;
    size_t convertedBufferLen=0;
    char* base64dest=NULL;
    size_t base64len=0;

    typedef void(*tyVecWriteFloatProc)(const t_ty&, float*);
    typedef void(*tyVecWriteByteProc)(const t_ty&, long*);

    MeshExportResult ret;
    xml_doc* xmldoc=doc;
    rapidxml::xml_node<>* streamnode=NULL;
    char tmpbuf[128];
    
    streamnode=xmldoc->allocate_node(node_element, "stream");
    rapidxml::xml_attribute<>* attr=xmldoc->allocate_attribute("semantic", getSemanticName(semantic));
    streamnode->append_attribute(attr);
    attr=xmldoc->allocate_attribute("type", getFormatName(typeformat));
    streamnode->append_attribute(attr);
    _itoa(semIdx, tmpbuf, 10);
    attr=xmldoc->allocate_attribute("index", xmldoc->allocate_string(tmpbuf));
    streamnode->append_attribute(attr);
    _itoa(elements, tmpbuf, 10);
    attr=xmldoc->allocate_attribute("count", xmldoc->allocate_string(tmpbuf));
    streamnode->append_attribute(attr);
    _itoa(0, tmpbuf, 10);
    attr=xmldoc->allocate_attribute("sinput", xmldoc->allocate_string(tmpbuf));
    streamnode->append_attribute(attr);

    tyVecWriteFloatProc vecwriter=NULL;
    tyVecWriteByteProc  colwriter=NULL;
    size_t destinc;
    float* fdest;
    long*  ldest;
    switch(typeformat){
    case Heart::eIF_FLOAT1: destinc=1; vecwriter=axisswap ? writeVecToFloatSwap<t_ty, t_indim, 1> : writeVecToFloat<t_ty, t_indim, 1>; break;
    case Heart::eIF_FLOAT2: destinc=2; vecwriter=axisswap ? writeVecToFloatSwap<t_ty, t_indim, 2> : writeVecToFloat<t_ty, t_indim, 2>; break;
    case Heart::eIF_FLOAT3: destinc=3; vecwriter=axisswap ? writeVecToFloatSwap<t_ty, t_indim, 3> : writeVecToFloat<t_ty, t_indim, 3>; break;
    case Heart::eIF_FLOAT4: destinc=4; vecwriter=axisswap ? writeVecToFloatSwap<t_ty, t_indim, 4> : writeVecToFloat<t_ty, t_indim, 4>; break;
    case Heart::eIF_UBYTE4_UNORM: destinc=1; colwriter=writeVecToByteUnorm<t_ty>; break;
    case Heart::eIF_UBYTE4_SNORM: destinc=1; colwriter=writeVecToByteSnorm<t_ty>; break;
    default: return NULL;
    }
    if (vecwriter) {
        fdest=new float[elements*destinc];
        convertedBuffer=boost::shared_array<char>((char*)fdest);
        convertedBufferLen=sizeof(float)*destinc*elements;
        for (size_t v=0,vn=elements; v<vn; ++v, fdest+=destinc) {
            vecwriter(inputbuf[v], fdest);
        }
    } else if (colwriter) {
        ldest=new long[elements*destinc];
        convertedBuffer=boost::shared_array<char>((char*)ldest);
        convertedBufferLen=sizeof(long)*destinc*elements;
        for (size_t v=0,vn=elements; v<vn; ++v, ldest+=destinc) {
            colwriter(inputbuf[v], ldest);
        }
    }


    base64len=Heart::hBase64::EncodeCalcRequiredSize(convertedBufferLen);
    base64dest=xmldoc->allocate_string(NULL, base64len);
    Heart::hBase64::Encode(convertedBuffer.get(), convertedBufferLen, base64dest, base64len);
    streamnode->value(base64dest, base64len);
    return streamnode;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MeshLodLevel::MeshLodLevel() 
    : aiScene_(NULL)
    , aiImporter_(new Assimp::Importer)
    , yzAxisSwap_(false)
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MeshLodLevel::~MeshLodLevel() {

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool MeshLodLevel::importMeshObject(const std::string& filepath, size_t lodlevel) {
    std::string matName;
    aiString aiMatName;
    char tmpbuf[32];
    lodlevel_=lodlevel;
    matNames_.clear();
    aiImporter_->FreeScene();
    aiScene_=NULL;
    aiScene_=aiImporter_->ReadFile(filepath, MESH_AI_FLAGS);
    if (!aiScene_) {
        return false;
    }
    meshSourceFilepath_=filepath;
    for(size_t m=0,nm=aiScene_->mNumMaterials; m<nm; ++m) {
        aiScene_->mMaterials[m]->Get(AI_MATKEY_NAME, aiMatName);
        _itoa(lodlevel, tmpbuf, 10);
        matName+=tmpbuf;
        matName+="_";
        matName+=aiMatName.C_Str();
        matNames_.push_back(matName);
        matName.clear();
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MeshExportResult MeshLodLevel::exportToMDF(xml_doc* xmldoc, rapidxml::xml_node<>* rootnode, vPackageSystem* pkgsys, const MaterialRemap& remap) const {
    using namespace rapidxml;

    MeshExportResult ret={true, ""};
    char tmpbuf[1024];

    for (size_t i=0,n=aiScene_->mNumMeshes; i<n; ++i) {
        rapidxml::xml_node<>* renderablenode=xmldoc->allocate_node(node_element, "renderable");
        aiString matname;
        std::string fullmatname;
        aiMesh* mesh=aiScene_->mMeshes[i];
        aiScene_->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_NAME, matname);
        _itoa(lodlevel_, tmpbuf, 10);
        fullmatname=tmpbuf;
        fullmatname+="_";
        fullmatname+=matname.C_Str();
        MaterialRemap::const_iterator matremap=remap.find(fullmatname);
        MDF_EXPORT_CHECK2(matremap!=remap.end(), "Failed to find remap for material ", matname.C_Str());

        std::string fullassetname=matremap->second;
        std::string pkgname=fullassetname.substr(0, fullassetname.find('.'));
        std::string resname=fullassetname.substr(fullassetname.find('.')+1);
        rapidxml::xml_attribute<>* matnameattr=xmldoc->allocate_attribute("material", xmldoc->allocate_string(fullassetname.c_str()));
        renderablenode->append_attribute(matnameattr);

        //Write out...
        boost::shared_array<char> idxBuffer;
        size_t convertedIdxBufferLen=0;
        size_t indices=mesh->mNumFaces*3;
        size_t indicessize=0;
        if (indices > 0xFFFF) {
            indicessize=indices*sizeof(hUint32);
            idxBuffer=boost::shared_array<char>(new char[indicessize]);
            hUint32* iptr=(hUint32*)idxBuffer.get();
            for (hUint32 faceIdx=0,n=mesh->mNumFaces; faceIdx < n; ++faceIdx) {
                hUint32 idxs[] = {
                    mesh->mFaces[faceIdx].mIndices[0],
                    mesh->mFaces[faceIdx].mIndices[1],
                    mesh->mFaces[faceIdx].mIndices[2],
                };
                //Only handle triangles
                MDF_EXPORT_CHECK1(mesh->mFaces[faceIdx].mNumIndices, "Non-Triangle mesh");
                *iptr=mesh->mFaces[faceIdx].mIndices[0]; ++iptr;
                *iptr=mesh->mFaces[faceIdx].mIndices[1]; ++iptr;
                *iptr=mesh->mFaces[faceIdx].mIndices[2]; ++iptr;
            }
        } else {
            indicessize=indices*sizeof(hUint16);
            idxBuffer=boost::shared_array<char>(new char[indicessize]);
            hUint16* iptr=(hUint16*)idxBuffer.get();
            for (hUint32 faceIdx=0,n=mesh->mNumFaces; faceIdx < n; ++faceIdx) {
                hUint16 idxs[] = {
                    (hUint16)mesh->mFaces[faceIdx].mIndices[0],
                    (hUint16)mesh->mFaces[faceIdx].mIndices[1],
                    (hUint16)mesh->mFaces[faceIdx].mIndices[2],
                };
                //Only handle triangles
                MDF_EXPORT_CHECK1(mesh->mFaces[faceIdx].mNumIndices, "Non-Triangle mesh");
                *iptr=mesh->mFaces[faceIdx].mIndices[0]; ++iptr;
                *iptr=mesh->mFaces[faceIdx].mIndices[1]; ++iptr;
                *iptr=mesh->mFaces[faceIdx].mIndices[2]; ++iptr;
            }
        }

        rapidxml::xml_node<>* idxstreamnode=NULL;
        idxstreamnode=xmldoc->allocate_node(node_element, "index");
        rapidxml::xml_attribute<>* idxattr=xmldoc->allocate_attribute("type", (indices > 0xFFFF) ? "IDX32" : "IDX16");
        idxstreamnode->append_attribute(idxattr);
        _itoa(indices, tmpbuf, 10);
        idxattr=xmldoc->allocate_attribute("count", xmldoc->allocate_string(tmpbuf));
        idxstreamnode->append_attribute(idxattr);
        size_t idxBase64len=Heart::hBase64::EncodeCalcRequiredSize(indicessize);
        char* idxBase64dest=xmldoc->allocate_string(NULL, idxBase64len);
        Heart::hBase64::Encode(idxBuffer.get(), indicessize, idxBase64dest, idxBase64len);
        idxstreamnode->value(idxBase64dest, idxBase64len);
        renderablenode->append_node(idxstreamnode);


        boost::shared_array<char> convertedBuffer;
        size_t convertedBufferLen=0;
        char* base64dest=NULL;
        size_t base64len=0;
        rapidxml::xml_node<>* streamnode=NULL;

        if (mesh->mVertices) {
            streamnode=writeStreamNode<aiVector3D, 3, false>(xmldoc, Heart::eIS_POSITION, Heart::eIF_FLOAT3, 0,
                mesh->mNumVertices, mesh->mVertices, yzAxisSwap_);
            MDF_EXPORT_CHECK1(streamnode, "Unsupported stream format");
            renderablenode->append_node(streamnode);
        }
        if (mesh->mNormals) {
            streamnode=writeStreamNode<aiVector3D, 3, false>(xmldoc, Heart::eIS_NORMAL, Heart::eIF_FLOAT3, 0,
                mesh->mNumVertices, mesh->mNormals, yzAxisSwap_);
            MDF_EXPORT_CHECK1(streamnode, "Unsupported stream format");
            renderablenode->append_node(streamnode);
        }
        if (mesh->mTangents) {
            streamnode=writeStreamNode<aiVector3D, 3, false>(xmldoc, Heart::eIS_TANGENT, Heart::eIF_FLOAT3, 0,
                mesh->mNumVertices, mesh->mNormals, yzAxisSwap_);
            MDF_EXPORT_CHECK1(streamnode, "Unsupported stream format");
            renderablenode->append_node(streamnode);
        }
        if (mesh->mBitangents) {
            streamnode=writeStreamNode<aiVector3D, 3, false>(xmldoc, Heart::eIS_BITANGENT, Heart::eIF_FLOAT3, 0,
                mesh->mNumVertices, mesh->mBitangents, yzAxisSwap_);
            MDF_EXPORT_CHECK1(streamnode, "Unsupported stream format");
            renderablenode->append_node(streamnode);
        }
        for (uint i=0; mesh->HasVertexColors(i); ++i) {
            streamnode=writeStreamNode<aiColor4D, 4, false>(xmldoc, Heart::eIS_COLOUR, Heart::eIF_FLOAT4, i,
                mesh->mNumVertices, mesh->mColors[i]);
            MDF_EXPORT_CHECK1(streamnode, "Unsupported stream format");
            renderablenode->append_node(streamnode);
        }
        for (uint i=0; mesh->HasTextureCoords(i); ++i) {
            streamnode=writeStreamNode<aiVector3D, 3, true>(xmldoc, Heart::eIS_TEXCOORD, Heart::eIF_FLOAT3, i,
                mesh->mNumVertices, mesh->mTextureCoords[i]);
            MDF_EXPORT_CHECK1(streamnode, "Unsupported stream format");
            renderablenode->append_node(streamnode);
        }

        rootnode->append_node(renderablenode);
    }

    return ret;
}

