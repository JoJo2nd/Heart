/********************************************************************

    filename:   mesh_lod_level.h  
    
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

#pragma once

#ifndef MESH_LOD_LEVEL_H__
#define MESH_LOD_LEVEL_H__

#include "material_utils.h"
#include "rapidxml/rapidxml.hpp"
#include "xml_helpers.h"
#include "boost/serialization/version.hpp"
#include "boost/serialization/split_member.hpp"
#include "boost/serialization/string.hpp"
#include "boost/smart_ptr.hpp"
#include <string>
#include <vector>

class MeshContainer;

class MeshLodLevel 
{
public:
    typedef std::vector< std::string > MaterialNameVector;
    MeshLodLevel();
    ~MeshLodLevel();

    bool                        importMeshObject(const std::string& filepath, size_t lodlevel);
    const MaterialNameVector&   getMaterialNames() const { return matNames_; }
    const aiScene*              getScene() const { return aiScene_; }
    const std::string&          getSceneName() const { return meshSourceFilepath_; }
    void                        setYZAxisSwap(bool v) { yzAxisSwap_=v; }
    bool                        getYZAxisSwap() const { return yzAxisSwap_; }
    MeshExportResult            exportToMDF( xml_doc* xmldoc, rapidxml::xml_node<>* rootnode, const MeshContainer& remap) const;
    std::string                 getMeshInfoString(const MeshContainer& meshcontainer) const;

private:
    friend class boost::serialization::access;
    BOOST_SERIALIZATION_SPLIT_MEMBER();
    template<typename t_archive>
    void save(t_archive& ar, const unsigned int version) const
    {
        ar&yzAxisSwap_;
        ar&lodlevel_;
        ar&matNames_;
        ar&meshSourceFilepath_;
    }
    template<typename t_archive>
    void load(t_archive& ar, const unsigned int version)
    {
        switch(version) {
        case 1: {
            ar&yzAxisSwap_;
        }
        case 0: {
            ar&lodlevel_;
            ar&matNames_;
            ar&meshSourceFilepath_;
        }
        }
        importMeshObject(meshSourceFilepath_, lodlevel_);
    }

    //non-serialised data
    boost::shared_ptr< Assimp::Importer >   aiImporter_; 
    const aiScene*                          aiScene_;
    //serialised data
    size_t              lodlevel_;
    MaterialNameVector  matNames_;
    std::string         meshSourceFilepath_;
    bool                yzAxisSwap_;
};

BOOST_CLASS_VERSION(MeshLodLevel, 1)

#endif // MESH_LOD_LEVEL_H__