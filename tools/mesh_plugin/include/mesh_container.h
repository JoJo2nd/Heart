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
#include "mesh_lod_level.h"
#include "viewer_api.h"
#include <vector>

class MeshContainer
{
public:

    MeshContainer();
    ~MeshContainer();
    const MaterialRemap&    getMaterialRemap() const { return materialRemap_; }
    void                    setMaterialRemap(const std::vector<std::string>& srcnames, const std::string& destname);
    size_t                  getLodCount() const { return lodLevels_.size(); }
    const MeshLodLevel&     getLodLevel(size_t level) const { return lodLevels_[level]; }
    bool                    importMeshIntoLod(size_t lodlvl, const std::string& filepath);
    void                    appendNewLodLevel();
    const std::string&      getExportPackageName() const { return outputPackageName_; }
    void                    setExportPackageName(const char* pkgname) { outputPackageName_=pkgname; }
    const std::string&      getExportResourceName() const { return outputResName_; }
    void                    setExportResourceName(const char* resname) { outputResName_=resname; }
    MeshExportResult        exportToMDF(const std::string& filepath, vPackageSystem* pakSys) const;

private:

    friend class boost::serialization::access;
    BOOST_SERIALIZATION_SPLIT_MEMBER();

    void removeMaterialsFromLod(const MeshLodLevel* mesh);
    void addMaterialsFromLod(const MeshLodLevel* mesh);
    template<typename t_archive>
    void save(t_archive& ar, const unsigned int version) const
    {
        ar&outputPackageName_;
        ar&outputResName_;
        ar&materialRemap_;
        ar&lodLevels_;
    }
    template<typename t_archive>
    void load(t_archive& ar, const unsigned int version)
    {
        switch(version) {
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
};

#endif // MESH_CONTAINER_H__