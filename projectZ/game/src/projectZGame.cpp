/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "projectZGame.h"
#include "2d\hRenderPlane2D.h"
#include "tileTextureSet.h"

hRegisterObjectType(ProjectZGame, ProjectZGame, projectZ::proto::projectZGame);

ProjectZGame::ProjectZGame(projectZ::proto::projectZGame* obj) {
    if (obj->has_zombiecount()) zombieCount = obj->zombiecount();
    if (obj->has_maptilecount()) mapTileCount = obj->maptilecount();
    if (obj->has_baserenderplanecomponentid()) basePlaneCompID = obj->baserenderplanecomponentid();
    if (obj->has_maptilelevelresource()) mapTilesResourceID = Heart::hStringID(obj->maptilelevelresource().c_str());
    maleZombiePrototypes.reserve(obj->malezombieentities_size());
    for (hInt i = 0, n = obj->malezombieentities_size(); i < n; ++i) {
        maleZombiePrototypes.push_back(Heart::hUUID::fromString(obj->malezombieentities(i).c_str(), obj->malezombieentities(i).size()));
    }
    femaleZombiePrototypes.reserve(obj->malezombieentities_size());
    for (hInt i = 0, n = obj->femalezombieentities_size(); i < n; ++i) {
        femaleZombiePrototypes.push_back(Heart::hUUID::fromString(obj->femalezombieentities(i).c_str(), obj->femalezombieentities(i).size()));
    }
}

hBool ProjectZGame::serialiseObject(projectZ::proto::projectZGame* obj) const {
    hChar guid_buffer[64];
    obj->set_zombiecount(zombieCount);
    obj->set_maptilecount(mapTileCount);
    obj->set_maptilelevelresource(mapTilesResourceID.c_str());
    obj->set_baserenderplanecomponentid(basePlaneCompID);
    for (hSize_t i=0, n=maleZombiePrototypes.size(); i<n; ++i) {
        Heart::hUUID::toString(maleZombiePrototypes[i], guid_buffer, sizeof(guid_buffer));
        obj->add_malezombieentities(guid_buffer, (hInt)i);
    }
    for (hSize_t i = 0, n = femaleZombiePrototypes.size(); i < n; ++i) {
        Heart::hUUID::toString(femaleZombiePrototypes[i], guid_buffer, sizeof(guid_buffer));
        obj->add_femalezombieentities(guid_buffer, (hInt)i);
    }
    return hTrue;
}

hBool ProjectZGame::linkObject() {
    hBool linked = true;
    if (!basePlane) {
        basePlane = getOwner()->getComponent<Heart::hRenderPlane2D>(basePlaneCompID);
        linked &= !!basePlane;
    }
    return linked;
}

hBool ProjectZGame::registerComponent()
{
    if (ProjectZGame::auto_object_registered) {
        Heart::hEntityFactory::hComponentMgt c = {
            Heart::hObjectFactory::getObjectDefinition(ProjectZGame::getTypeNameStatic()),
            [](Heart::hEntity* owner, Heart::hObjectMarshall* marshall) -> Heart::hEntityComponent* { 
                auto* real_marshall = static_cast<ProjectZGame::MarshallType*>(marshall);
                return new ProjectZGame(real_marshall);
            },
            [](Heart::hEntityComponent* ptr) { delete ptr; }
        };
        Heart::hEntityFactory::registerComponentManagement(c);
    }
    if (TileTextureSet::auto_object_registered) {
        Heart::hEntityFactory::hComponentMgt c = {
            Heart::hObjectFactory::getObjectDefinition(TileTextureSet::getTypeNameStatic()),
            [](Heart::hEntity* owner, Heart::hObjectMarshall* marshall) -> Heart::hEntityComponent* {
                auto* real_marshall = static_cast<TileTextureSet::MarshallType*>(marshall);
                return new TileTextureSet(real_marshall);
            },
            [](Heart::hEntityComponent* ptr) { delete ptr; }
        };
        Heart::hEntityFactory::registerComponentManagement(c);
    }
    return hTrue;
}
