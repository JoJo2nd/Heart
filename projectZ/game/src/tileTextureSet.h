/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "base/hUUID.h"
#include "base/hStringID.h"
#include "components/hObjectFactory.h"
#include "components/hEntity.h"
#include "components/hEntityFactory.h"
#include "2d/hDynamicTileSet2D.h"
#include "tileset.pb.h"
#include <memory>

namespace Heart {
    class hDynamicTileSet2D;
    class hTextureResource;
}

class TileTextureSet : public Heart::hEntityComponent {
    struct LoadLinkInfo { // Structure to store resource IDs & entity guids needed to load.
        hUuid_t dynamicTextureSet;
        std::vector<Heart::hStringID> northRoadResourceIDs;
        std::vector<Heart::hStringID> eastRoadResourceIDs;
        std::vector<Heart::hStringID> southRoadResourceIDs;
        std::vector<Heart::hStringID> westRoadResourceIDs;
        std::vector<Heart::hStringID> intersectionRoadResourceIDs;
        std::vector<Heart::hStringID> nonRoadResourceIDs;
    };
    struct Tile {
        Heart::hTextureResource* resource = nullptr;
        Heart::hDynamicTileSet2D::TileHandle handle = Heart::hDynamicTileSet2D::InvalidTileHandle;
    };

    std::unique_ptr<LoadLinkInfo> linkingInfo;
    Heart::hDynamicTileSet2D* tileSet = nullptr;
    std::vector<Tile> northRoadResources;
    std::vector<Tile> eastRoadResources;
    std::vector<Tile> southRoadResources;
    std::vector<Tile> westRoadResources;
    std::vector<Tile> intersectionRoadResources;
    std::vector<Tile> nonRoadResources;
public:
    hObjectType(TileTextureSet, projectZ::proto::projectZTileSet);

};