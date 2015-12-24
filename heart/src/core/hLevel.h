/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "core/hProtobuf.h"
#include "components/hObjectFactory.h"
#include "entity_def.pb.h"
#include <vector>

namespace Heart {
    class hEntity;

class hLevel {
    static std::vector<hLevel*> loadedLevels;

public:
    hObjectType(hLevel, Heart::proto::LevelDefinition);

    ~hLevel();

    std::string levelName;
    std::vector<hEntity*> levelEntities;
};

}