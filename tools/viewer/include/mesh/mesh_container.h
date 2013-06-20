/********************************************************************

    filename:   mesh_container.h  
    
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

#ifndef MESH_CONTAINER_H__
#define MESH_CONTAINER_H__

#include "boost/serialization/version.hpp"
#include "boost/serialization/split_member.hpp"
#include "boost/serialization/string.hpp"
#include "boost/serialization/map.hpp"
#include "boost/serialization/vector.hpp"
#include "mesh/mesh_lod_level.h"
#include <vector>

class MeshContainer
{
public:

    MeshContainer();
    ~MeshContainer();
    const MaterialRemap&    getMaterialRemap() const { return materialRemap_; }
    const std::string&      getDefaultMaterialRemap() const { return defaultMaterialResource_; }
    const std::string&      getMaterialRemap(const char* binding) const {
        auto it = materialRemap_.find(binding);
        return (it == materialRemap_.end()) ? defaultMaterialResource_ : it->second;
    }
    void                    importMaterialRemapBindings(const std::string& filepath, bool clearprevmappings);
    size_t                  getLodCount() const { return lodLevels_.size(); }
    const MeshLodLevel&     getLodLevel(size_t level) const { return lodLevels_[level]; }
    bool                    importMeshIntoLod(size_t lodlvl, const std::string& filepath);
    void                    appendNewLodLevel();
    void                    setYZAxisSwap(size_t lodlvl, bool val);
    MeshExportResult        exportToMDF(const std::string& filepath) const;
    std::string             getMeshInfoString() const;

private:

    friend class boost::serialization::access;
    BOOST_SERIALIZATION_SPLIT_MEMBER();

    void removeMaterialsFromLod(const MeshLodLevel* mesh);
    void addMaterialsFromLod(const MeshLodLevel* mesh);
    template<typename t_archive>
    void save(t_archive& ar, const unsigned int version) const
    {
        ar&defaultMaterialResource_;
        ar&outputPackageName_;
        ar&outputResName_;
        ar&materialRemap_;
        ar&lodLevels_;
    }
    template<typename t_archive>
    void load(t_archive& ar, const unsigned int version)
    {
        switch(version) {
        case 1: {
            ar&defaultMaterialResource_;
        }
        case 0: {
            ar&outputPackageName_;
            ar&outputResName_;
            ar&materialRemap_;
            ar&lodLevels_;
        }
        }
    }

    //serialised 
    std::string                             outputPackageName_;
    std::string                             outputResName_;
    std::map< std::string, std::string >    materialRemap_;
    std::vector< MeshLodLevel >             lodLevels_;
    std::string                             defaultMaterialResource_;
};

BOOST_CLASS_VERSION(MeshContainer, 1)

#endif // MESH_CONTAINER_H__