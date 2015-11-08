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

class ProjectZGame : public Heart::hEntityComponent {
    hUint32 zombieCount;
    hUint32 mapTileCount;
    Heart::hStringID mapTilesResourceID;
    std::vector<hUuid_t> maleZombiePrototypes;
    std::vector<hUuid_t> femaleZombiePrototypes;

public:
    hObjectType(ProjectZGame, projectZ::proto::projectZGame);

    static hBool registerComponent();
};