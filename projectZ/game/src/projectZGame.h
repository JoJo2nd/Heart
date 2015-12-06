/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "base/hUUID.h"
#include "components/hObjectFactory.h"
#include "components/hEntity.h"
#include "components/hEntityFactory.h"
#include "projectz.pb.h"

namespace Heart {
    class hRenderPlane2D;
}

class ProjectZGame : public Heart::hEntityComponent {
    hUint32 zombieCount = 0;
    hUint32 mapTileCount = 0;
    Heart::hStringID mapTilesResourceID;
    std::vector<hUuid_t> maleZombiePrototypes;
    std::vector<hUuid_t> femaleZombiePrototypes;
    hUint32 basePlaneCompID = 0;
    hUint32 spritePlaneCompID = 0;
    Heart::hRenderPlane2D* basePlane = nullptr;
public:
    hObjectType(ProjectZGame, projectZ::proto::projectZGame);

    static hBool registerComponent();
};