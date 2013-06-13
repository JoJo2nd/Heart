/********************************************************************

    filename:   mesh_container.cpp  
    
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
#include "mesh/mesh_container.h"
#include <fstream>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MeshContainer::MeshContainer() {

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MeshContainer::~MeshContainer() {

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshContainer::setMaterialRemap(const std::vector<std::string>& srcnames, const std::string& destname) {
    std::map< std::string, std::string >::const_iterator nm=materialRemap_.end();
    for (size_t i=0,n=srcnames.size(); i<n; ++i) {
        if (materialRemap_.find(srcnames[i]) != nm) {
            materialRemap_[srcnames[i]]=destname;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshContainer::appendNewLodLevel() {
    lodLevels_.resize(lodLevels_.size()+1);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshContainer::setYZAxisSwap(size_t lodlvl, bool val) {
    lodLevels_[lodlvl].setYZAxisSwap(val);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool MeshContainer::importMeshIntoLod(size_t lodlvl, const std::string& filepath) {
    bool succ;
    removeMaterialsFromLod(&lodLevels_[lodlvl]);
    succ=lodLevels_[lodlvl].importMeshObject(filepath, lodlvl);
    if (succ) {
        addMaterialsFromLod(&lodLevels_[lodlvl]); 
    }
    return succ;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshContainer::removeMaterialsFromLod(const MeshLodLevel* mesh) {
    for (size_t m=0,nm=mesh->getMaterialNames().size(); m<nm; ++m) {
        materialRemap_.erase(mesh->getMaterialNames()[m]);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshContainer::addMaterialsFromLod(const MeshLodLevel* mesh) {
    for (size_t m=0,nm=mesh->getMaterialNames().size(); m<nm; ++m) {
        materialRemap_.insert(std::pair<std::string, std::string>(mesh->getMaterialNames()[m], ""));
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MeshExportResult MeshContainer::exportToMDF(const std::string& filepath, vPackageSystem* pakSys) const {
    using namespace rapidxml;

    MeshExportResult res={false, ""};
    char tmpbuf[1024];
    xml_doc outputxml;
    
    xml_node<>* rootnode = outputxml.allocate_node(node_element, "modeldescription");
    for (size_t i=0,n=lodLevels_.size(); i<n; ++i) {
        //lvl attribute
        _itoa(i, tmpbuf, 10);
        xml_node<>* lodnode=outputxml.allocate_node(node_element, "lod");
        char* lvlattstr=outputxml.allocate_string(tmpbuf);
        xml_attribute<>* lvlatt=outputxml.allocate_attribute("level", lvlattstr);
        lodnode->append_attribute(lvlatt);
        //range attribute
        _itoa((i+1)*1000, tmpbuf, 10);
        lvlattstr=outputxml.allocate_string(tmpbuf);
        lvlatt=outputxml.allocate_attribute("range", lvlattstr);
        lodnode->append_attribute(lvlatt);

        rootnode->append_node(lodnode);
        res=lodLevels_[i].exportToMDF(&outputxml, lodnode, pakSys, materialRemap_);
        if (!res.exportOK) {
            return res;
        }
    }

    outputxml.append_node(rootnode);

    std::fstream file;
    file.open(filepath, std::ios_base::out|std::ios_base::binary|std::ios_base::trunc);
    if (!file.is_open()) {
        res.exportOK=false;
        res.errors="Couldn't open output file for write";
        return res;
    }
    file << outputxml;
    file.close();

    return res;
}
