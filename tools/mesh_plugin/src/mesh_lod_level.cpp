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

#include "mesh_lod_level.h"
#include "boost/filesystem.hpp"
#include <sstream>
#include "material_utils.h"

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
#define AI_COlOUR_TO_UBYTE(c) \
    (long)((((unsigned char)(c.r*256.f))<<24)|(((unsigned char)(c.b*256.f))<<16)|(((unsigned char)(c.b*256.f))<<8)|(((unsigned char)(c.a*256.f))))

#define AI_COlOUR_TO_SBYTE(c) \
    (long)((((unsigned char)(c.r*512.f-256.f))<<24)|(((unsigned char)(c.b*512.f-256.f))<<16)|(((unsigned char)(c.b*512.f-256.f))<<8)|(((unsigned char)(c.a*512.f-256.f))))

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
const char* getSemanticName(Heart::hInputSemantic sem) {
    switch(sem) {
    case Heart::eIS_POSITION: return "POSITION";
    case Heart::eIS_NORMAL: return "NORMAL";
    case Heart::eIS_TEXCOORD: return "TEXCOORD";
    case Heart::eIS_COLOUR: return "COLOUR";
    case Heart::eIS_TANGENT: return "TANGENT";
    case Heart::eIS_BITANGENT: return "BITANGENT";
    case Heart::eIS_INSTANCE: return "INSTANCE";
    }
    return "UNKNOWN";
}

const char* getFormatName(Heart::hInputFormat sem) {
    switch(sem) {
    case Heart::eIF_FLOAT4: return "FLOAT4";
    case Heart::eIF_FLOAT3: return "FLOAT3";
    case Heart::eIF_FLOAT2: return "FLOAT2";
    case Heart::eIF_FLOAT1: return "FLOAT1";
    case Heart::eIF_UBYTE4_UNORM: return "UBTYE4UNORM";
    case Heart::eIF_UBYTE4_SNORM: return "UBTYE4SNORM";
    }
    return "UNKNOWN";
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
typedef void(*vecWriteProc)(const aiVector3D&, float*);
typedef void(*colWriteFloatProc)(const aiColor4D&, float*);
typedef void(*colWriteByteProc)(const aiColor4D&, long*);

void writeVecToFloat(const aiVector3D& v, float* out) {
    out[0]=v.x;
}

void writeVecTo2Floats(const aiVector3D& v, float* out) {
    out[0]=v.x;
    out[1]=v.y;
}

void writeVecTo3Floats(const aiVector3D& v, float* out) {
    out[0]=v.x;
    out[1]=v.y;
    out[2]=v.z;
}

void writeVecTo4Floats(const aiVector3D& v, float* out) {
    out[0]=v.x;
    out[1]=v.y;
    out[2]=v.z;
    out[3]=1.f;
}

void writeColourToFloat(const aiColor4D& v, float* out) {
    out[0]=v.r;
}

void writeColourTo2Floats(const aiColor4D& v, float* out) {
    out[0]=v.r;
    out[1]=v.g;
}

void writeColourTo3Floats(const aiColor4D& v, float* out) {
    out[0]=v.r;
    out[1]=v.g;
    out[2]=v.b;
}

void writeColourTo4Floats(const aiColor4D& v, float* out) {
    out[0]=v.r;
    out[1]=v.g;
    out[2]=v.b;
    out[3]=v.a;
}

void writeColourToByteUnorm(const aiColor4D& v, long* out) {
    out[0]=AI_COlOUR_TO_UBYTE(v);
}

void writeColourToByteSnorm(const aiColor4D& v, long* out) {
    out[0]=AI_COlOUR_TO_SBYTE(v);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MeshLodLevel::MeshLodLevel() 
    : aiScene_(NULL)
    , aiImporter_(new Assimp::Importer)
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
        if (matremap==remap.end()) {
            ret.exportOK=false;
            ret.errors="Failed to find remap for material ";
            ret.errors+=matname.C_Str();
            return ret;
        }

        std::string fullassetname=matremap->second;
        std::string pkgname=fullassetname.substr(0, fullassetname.find('.'));
        std::string resname=fullassetname.substr(fullassetname.find('.')+1);
        vPackage* pkg=pkgsys->getPackage(pkgname.c_str());
        if (!pkg) {
            ret.exportOK=false;
            ret.errors="Failed to remap material, couldn't find package ";
            ret.errors+=pkgname.c_str();
            return ret;
        }
        vResource* res=pkg->getResourceInfoByName(resname.c_str());
        if (!res) {
            ret.exportOK=false;
            ret.errors="Failed to remap material, couldn't find resource ";
            ret.errors+=fullassetname.c_str();
            return ret;
        }
        rapidxml::xml_attribute<>* matnameattr=xmldoc->allocate_attribute("material", xmldoc->allocate_string(fullassetname.c_str()));
        renderablenode->append_attribute(matnameattr);

        std::ifstream infile;
        infile.open(res->getInputFilePath());
        if(!infile.is_open()) {
            ret.exportOK=false;
            ret.errors="Failed to remap material, couldn't open resource ";
            ret.errors+=fullassetname.c_str();
            ret.errors+=" with filepath ";
            ret.errors+=res->getInputFilePath();
            return ret;
        }
        std::vector< std::string > vertexProgs;
        size_t xmlfilelen=boost::filesystem::file_size(res->getInputFilePath());
        boost::shared_array<char> xmldata(new char[xmlfilelen+1]);
        memset(xmldata.get(), 0, xmlfilelen+1);
        infile.read(xmldata.get(), xmlfilelen);
        xmldata[xmlfilelen]=0;
        extractVertexProgramsFromMaterial(xmldata.get(), xmlfilelen, &vertexProgs);

        if (vertexProgs.empty()) {
            ret.exportOK=false;
            ret.errors="Failed to any vertex programs in material ";
            ret.errors+=fullassetname.c_str();
            return ret;
        }

        std::vector< Heart::hInputLayoutDesc > finalinputlayout;
        for (size_t vp=0,vpn=vertexProgs.size(); vp<vpn; ++vp) {
            std::string vppkgname=vertexProgs[i].substr(0, vertexProgs[i].find('.'));
            std::string vpresname=vertexProgs[i].substr(vertexProgs[i].find('.')+1);
            vPackage* vppkg=pkgsys->getPackage(vppkgname.c_str());
            if (!vppkg) {
                ret.exportOK=false;
                ret.errors="Failed to read vertex prog, couldn't find package ";
                ret.errors+=vppkgname;
                return ret;
            }
            vResource* vpres=pkg->getResourceInfoByName(vpresname.c_str());
            if (!vpres) {
                ret.exportOK=false;
                ret.errors="Failed to read vertex prog, couldn't find resource ";
                ret.errors+=vertexProgs[i];
                return ret;
            }
            std::ifstream vpfile;
            vpfile.open(vpres->getInputFilePath());
            if(!vpfile.is_open()) {
                ret.exportOK=false;
                ret.errors="Failed to read vertex prog, couldn't open resource ";
                ret.errors+=vertexProgs[i];
                ret.errors+=" with filepath ";
                ret.errors+=vpres->getInputFilePath();
                return ret;
            }
            std::vector< Heart::hInputLayoutDesc > inputlayout;
            size_t vertexSrcLen=boost::filesystem::file_size(vpres->getInputFilePath());
            boost::shared_array<char> vertexSrc(new char[vertexSrcLen+1]);
            memset(vertexSrc.get(), 0, vertexSrcLen+1);
            vpfile.read(vertexSrc.get(), vertexSrcLen);
            if (!vpres->getParameter("PROFILE") || !vpres->getParameter("ENTRY")) {
                ret.exportOK=false;
                ret.errors="Failed to read vertex prog, couldn't find entry or profile";
                return ret;
            }
            bool succ=extractVertexInputLayoutFromShaderSource(
                vertexSrc.get(), 
                strlen(vertexSrc.get()), 
                vpres->getParameter("PROFILE")->getValue(),
                vpres->getParameter("ENTRY")->getValue(),
                vpres->getInputFilePath(), vppkg->getRootPath(),
                &inputlayout, &ret.errors);
            if (!succ) {
                return ret;
            }
            for (size_t il=0,iln=inputlayout.size(); il<iln; ++il) {
                bool alreadyAdded=false;
                for (size_t fil=0,filn=finalinputlayout.size(); fil<filn; ++fil) {
                    if (inputlayout[il].semantic_==finalinputlayout[fil].semantic_ &&
                        inputlayout[il].semIndex_==finalinputlayout[fil].semIndex_) {
                            alreadyAdded=true;
                            break;
                    }
                }
                if (!alreadyAdded) {
                    finalinputlayout.push_back(inputlayout[il]);
                }
            }
        }

        //Write out...
        for (size_t i=0,n=finalinputlayout.size(); i<n; ++i) {
            boost::shared_array<char> convertedBuffer(NULL);
            size_t convertedBufferLen=0;
            Heart::hInputLayoutDesc inSem=finalinputlayout[i];
            char* base64dest=NULL;
            size_t base64len=0;
            rapidxml::xml_node<>* streamnode=NULL;

            if (inSem.semantic_==Heart::eIS_POSITION) {
                if (inSem.semIndex_ > 0) {
                    ret.exportOK=false;
                    ret.errors="Unsupported position semantic index (value too high)";
                    return ret;
                }
                streamnode=xmldoc->allocate_node(node_element, "stream");
                rapidxml::xml_attribute<>* attr=xmldoc->allocate_attribute("semantic", getSemanticName(inSem.semantic_));
                streamnode->append_attribute(attr);
                attr=xmldoc->allocate_attribute("type", getFormatName(inSem.typeFormat_));
                streamnode->append_attribute(attr);
                _itoa(inSem.semIndex_, tmpbuf, 10);
                attr=xmldoc->allocate_attribute("index", xmldoc->allocate_string(tmpbuf));
                streamnode->append_attribute(attr);
                _itoa(mesh->mNumVertices, tmpbuf, 10);
                attr=xmldoc->allocate_attribute("count", xmldoc->allocate_string(tmpbuf));
                streamnode->append_attribute(attr);

                vecWriteProc writer;
                size_t destinc;
                float* dest;
                switch(inSem.typeFormat_){
                case Heart::eIF_FLOAT1: destinc=1; writer=writeVecToFloat; break;
                case Heart::eIF_FLOAT2: destinc=2; writer=writeVecTo2Floats; break;
                case Heart::eIF_FLOAT3: destinc=3; writer=writeVecTo3Floats; break;
                case Heart::eIF_FLOAT4: destinc=4; writer=writeVecTo4Floats; break;
                default: 
                    ret.exportOK=false; 
                    ret.errors="Unsupported position format";
                    return ret;
                }
                dest=new float[mesh->mNumVertices*destinc];
                convertedBuffer=boost::shared_array<char>((char*)dest);
                convertedBufferLen=sizeof(float)*destinc*mesh->mNumVertices;
                for (size_t v=0,vn=mesh->mNumVertices; v<vn; ++v, dest+=destinc) {
                    writer(mesh->mVertices[v], dest);
                }
            } else if (inSem.semantic_==Heart::eIS_NORMAL) {
                if (inSem.semIndex_ > 0) {
                    ret.exportOK=false;
                    ret.errors="Unsupported normal semantic index (value too high)";
                    return ret;
                }
                streamnode=xmldoc->allocate_node(node_element, "stream");
                rapidxml::xml_attribute<>* attr=xmldoc->allocate_attribute("semantic", getSemanticName(inSem.semantic_));
                streamnode->append_attribute(attr);
                attr=xmldoc->allocate_attribute("type", getFormatName(inSem.typeFormat_));
                streamnode->append_attribute(attr);
                _itoa(inSem.semIndex_, tmpbuf, 10);
                attr=xmldoc->allocate_attribute("index", xmldoc->allocate_string(tmpbuf));
                streamnode->append_attribute(attr);
                _itoa(mesh->mNumVertices, tmpbuf, 10);
                attr=xmldoc->allocate_attribute("count", xmldoc->allocate_string(tmpbuf));
                streamnode->append_attribute(attr);

                vecWriteProc writer;
                size_t destinc;
                float* dest;
                switch(inSem.typeFormat_){
                case Heart::eIF_FLOAT1: destinc=1; writer=writeVecToFloat; break;
                case Heart::eIF_FLOAT2: destinc=2; writer=writeVecTo2Floats; break;
                case Heart::eIF_FLOAT3: destinc=3; writer=writeVecTo3Floats; break;
                case Heart::eIF_FLOAT4: destinc=4; writer=writeVecTo4Floats; break;
                default: 
                    ret.exportOK=false; 
                    ret.errors="Unsupported normal format";
                    return ret;
                }
                dest=new float[mesh->mNumVertices*destinc];
                convertedBuffer=boost::shared_array<char>((char*)dest);
                convertedBufferLen=sizeof(float)*destinc*mesh->mNumVertices;
                for (size_t v=0,vn=mesh->mNumVertices; v<vn; ++v, dest+=destinc) {
                    writer(mesh->mNormals[v], dest);
                }
            } else if (inSem.semantic_==Heart::eIS_TANGENT) {
                if (inSem.semIndex_ > 0) {
                    ret.exportOK=false;
                    ret.errors="Unsupported tangent semantic index (value too high)";
                    return ret;
                }
                streamnode=xmldoc->allocate_node(node_element, "stream");
                rapidxml::xml_attribute<>* attr=xmldoc->allocate_attribute("semantic", getSemanticName(inSem.semantic_));
                streamnode->append_attribute(attr);
                attr=xmldoc->allocate_attribute("type", getFormatName(inSem.typeFormat_));
                streamnode->append_attribute(attr);
                _itoa(inSem.semIndex_, tmpbuf, 10);
                attr=xmldoc->allocate_attribute("index", xmldoc->allocate_string(tmpbuf));
                streamnode->append_attribute(attr);
                _itoa(mesh->mNumVertices, tmpbuf, 10);
                attr=xmldoc->allocate_attribute("count", xmldoc->allocate_string(tmpbuf));
                streamnode->append_attribute(attr);

                vecWriteProc writer;
                size_t destinc;
                float* dest;
                switch(inSem.typeFormat_){
                case Heart::eIF_FLOAT1: destinc=1; writer=writeVecToFloat; break;
                case Heart::eIF_FLOAT2: destinc=2; writer=writeVecTo2Floats; break;
                case Heart::eIF_FLOAT3: destinc=3; writer=writeVecTo3Floats; break;
                case Heart::eIF_FLOAT4: destinc=4; writer=writeVecTo4Floats; break;
                default: 
                    ret.exportOK=false; 
                    ret.errors="Unsupported tangent format";
                    return ret;
                }
                dest=new float[mesh->mNumVertices*destinc];
                convertedBuffer=boost::shared_array<char>((char*)dest);
                convertedBufferLen=sizeof(float)*destinc*mesh->mNumVertices;
                for (size_t v=0,vn=mesh->mNumVertices; v<vn; ++v, dest+=destinc) {
                    writer(mesh->mTangents[v], dest);
                }
            } else if (inSem.semantic_==Heart::eIS_COLOUR) {
                if (inSem.semIndex_ > 3 || !mesh->HasVertexColors(inSem.semIndex_)) {
                    ret.exportOK=false;
                    ret.errors="Unsupported colour semantic index (value too high or doesn't exist in mesh)";
                    return ret;
                }
                streamnode=xmldoc->allocate_node(node_element, "stream");
                rapidxml::xml_attribute<>* attr=xmldoc->allocate_attribute("semantic", getSemanticName(inSem.semantic_));
                streamnode->append_attribute(attr);
                attr=xmldoc->allocate_attribute("type", getFormatName(inSem.typeFormat_));
                streamnode->append_attribute(attr);
                _itoa(inSem.semIndex_, tmpbuf, 10);
                attr=xmldoc->allocate_attribute("index", xmldoc->allocate_string(tmpbuf));
                streamnode->append_attribute(attr);
                _itoa(mesh->mNumVertices, tmpbuf, 10);
                attr=xmldoc->allocate_attribute("count", xmldoc->allocate_string(tmpbuf));
                streamnode->append_attribute(attr);

                colWriteFloatProc vecwriter=NULL;
                colWriteByteProc  colwriter=NULL;
                size_t destinc;
                float* fdest;
                long*  ldest;
                switch(inSem.typeFormat_){
                case Heart::eIF_FLOAT1: destinc=1; vecwriter=writeColourToFloat; break;
                case Heart::eIF_FLOAT2: destinc=2; vecwriter=writeColourTo2Floats; break;
                case Heart::eIF_FLOAT3: destinc=3; vecwriter=writeColourTo3Floats; break;
                case Heart::eIF_FLOAT4: destinc=4; vecwriter=writeColourTo4Floats; break;
                case Heart::eIF_UBYTE4_UNORM: destinc=1; colwriter=writeColourToByteUnorm; break;
                case Heart::eIF_UBYTE4_SNORM: destinc=1; colwriter=writeColourToByteSnorm; break;
                default: 
                    ret.exportOK=false; 
                    ret.errors="Unsupported color format";
                    return ret;
                }
                if (vecwriter) {
                    fdest=new float[mesh->mNumVertices*destinc];
                    convertedBuffer=boost::shared_array<char>((char*)fdest);
                    convertedBufferLen=sizeof(float)*destinc*mesh->mNumVertices;
                    for (size_t v=0,vn=mesh->mNumVertices; v<vn; ++v, fdest+=destinc) {
                        vecwriter(mesh->mColors[inSem.semIndex_][v], fdest);
                    }
                } else if (colwriter) {
                    ldest=new long[mesh->mNumVertices*destinc];
                    convertedBuffer=boost::shared_array<char>((char*)ldest);
                    convertedBufferLen=sizeof(long)*destinc*mesh->mNumVertices;
                    for (size_t v=0,vn=mesh->mNumVertices; v<vn; ++v, ldest+=destinc) {
                        colwriter(mesh->mColors[inSem.semIndex_][v], ldest);
                    }
                }
            }

            base64len=Heart::hBase64::EncodeCalcRequiredSize(convertedBufferLen);
            base64dest=xmldoc->allocate_string(NULL, base64len);
            Heart::hBase64::Encode(convertedBuffer.get(), convertedBufferLen, base64dest, base64len);
            streamnode->value(base64dest, base64len);
            renderablenode->append_node(streamnode);
            
        }

        rootnode->append_node(renderablenode);
    }

    return ret;
}

