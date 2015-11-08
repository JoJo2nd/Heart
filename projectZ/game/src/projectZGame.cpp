/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "projectZGame.h"

hRegisterObjectType(ProjectZGame, ProjectZGame, projectZ::proto::projectZGame);

hBool ProjectZGame::serialiseObject(projectZ::proto::projectZGame* obj) const {
    hChar guid_buffer[64];
    obj->set_zombiecount(zombieCount);
    obj->set_maptilecount(mapTileCount);
    obj->set_maptilelevelresource(mapTilesResourceID.c_str());
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
hBool ProjectZGame::deserialiseObject(projectZ::proto::projectZGame* obj) {
    if (obj->has_zombiecount()) zombieCount = obj->zombiecount();
    if (obj->has_maptilecount()) mapTileCount = obj->maptilecount();
    if (obj->has_maptilelevelresource()) mapTilesResourceID = Heart::hStringID(obj->maptilelevelresource().c_str());
    maleZombiePrototypes.reserve(obj->malezombieentities_size());
    for (hInt i=0, n=obj->malezombieentities_size(); i<n; ++i) {
        maleZombiePrototypes.push_back(Heart::hUUID::fromString(obj->malezombieentities(i).c_str(), obj->malezombieentities(i).size()));
    }
    femaleZombiePrototypes.reserve(obj->malezombieentities_size());
    for (hInt i = 0, n = obj->femalezombieentities_size(); i < n; ++i) {
        femaleZombiePrototypes.push_back(Heart::hUUID::fromString(obj->femalezombieentities(i).c_str(), obj->femalezombieentities(i).size()));
    }
    return hTrue;
}
hBool ProjectZGame::linkObject() {
    return hTrue;
}

hBool ProjectZGame::registerComponent()
{
    Heart::hEntityFactory::hComponentMgt c = {
        Heart::hObjectFactory::getObjectDefinition(ProjectZGame::getTypeNameStatic()),
        [](Heart::hEntity* owner) -> Heart::hEntityComponent* { return new ProjectZGame(); },
        [](Heart::hEntityComponent* ptr) { delete ptr; }
    };
    Heart::hEntityFactory::registerComponentManagement(c);
    return ProjectZGame::auto_object_registered;
}
