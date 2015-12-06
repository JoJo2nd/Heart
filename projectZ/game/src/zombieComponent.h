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
#include "2d/hDynamicTileSet2D.h"
#include "zombie.pb.h"

namespace Heart {
class hTextureResource;
class hDynamicTileSet2D;
}

class ZombieComponent : public Heart::hEntityComponent {
    typedef projectZ::proto::ZombieType ZombieType;

    ZombieType type;
    Heart::hStringID textureResourceID;
    hUuid_t dynamicTileSetID;
    Heart::hTextureResource* texture = nullptr;
    Heart::hDynamicTileSet2D* tileSet = nullptr;
    Heart::hDynamicTileSet2D::TileHandle tile = Heart::hDynamicTileSet2D::InvalidTileHandle;

public:
    hObjectType(ZombieComponent, projectZ::proto::Zombie);

    ZombieComponent() 
        : texture(nullptr) {
    }

    static hBool registerComponent();
};