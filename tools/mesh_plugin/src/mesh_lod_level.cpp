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

MeshExportResult MeshLodLevel::exportToMDF(rapidxml::xml_node<>* rootnode, vPackageSystem* pkgsys, const MaterialRemap& remap) const {
    MeshExportResult ret={true, ""};
    char tmpbuf[1024];

    for (size_t i=0,n=aiScene_->mNumMeshes; i<n; ++i) {
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
    }

    return ret;
}

