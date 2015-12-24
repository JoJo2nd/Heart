/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "tileTextureSet.h"
#include "core/hResourceManager.h"
#include "render/hTextureResource.h"
#include "2d/hDynamicTileSet2D.h"

hRegisterObjectType(TileTextureSet, TileTextureSet, projectZ::proto::projectZTileSet);

TileTextureSet::TileTextureSet(projectZ::proto::projectZTileSet* obj) {
    linkingInfo.reset(new LoadLinkInfo());
    if (obj->has_dynamictilesetid()) linkingInfo->dynamicTextureSet = Heart::hUUID::fromString(obj->dynamictilesetid().c_str(), obj->dynamictilesetid().size());
    linkingInfo->northRoadResourceIDs.reserve(obj->northroad_size());
    northRoadResources.resize(obj->northroad_size());
    for (hUint i = 0, n = obj->northroad_size(); i < n; ++i) {
        linkingInfo->northRoadResourceIDs.push_back(Heart::hStringID(obj->northroad(i).c_str()));
    }
    linkingInfo->eastRoadResourceIDs.reserve(obj->eastroad_size());
    eastRoadResources.resize(obj->eastroad_size());
    for (hUint i = 0, n = obj->eastroad_size(); i < n; ++i) {
        linkingInfo->eastRoadResourceIDs.push_back(Heart::hStringID(obj->eastroad(i).c_str()));
    }
    linkingInfo->southRoadResourceIDs.reserve(obj->southroad_size());
    southRoadResources.resize(obj->southroad_size());
    for (hUint i = 0, n = obj->southroad_size(); i < n; ++i) {
        linkingInfo->southRoadResourceIDs.push_back(Heart::hStringID(obj->southroad(i).c_str()));
    }
    linkingInfo->westRoadResourceIDs.reserve(obj->westroad_size());
    westRoadResources.resize(obj->westroad_size());
    for (hUint i = 0, n = obj->westroad_size(); i < n; ++i) {
        linkingInfo->westRoadResourceIDs.push_back(Heart::hStringID(obj->westroad(i).c_str()));
    }
    linkingInfo->intersectionRoadResourceIDs.reserve(obj->intersectionroad_size());
    intersectionRoadResources.resize(obj->intersectionroad_size());
    for (hUint i = 0, n = obj->intersectionroad_size(); i < n; ++i) {
        linkingInfo->intersectionRoadResourceIDs.push_back(Heart::hStringID(obj->intersectionroad(i).c_str()));
    }
    linkingInfo->nonRoadResourceIDs.reserve(obj->nonroad_size());
    nonRoadResources.resize(obj->nonroad_size());
    for (hUint i = 0, n = obj->nonroad_size(); i < n; ++i) {
        linkingInfo->nonRoadResourceIDs.push_back(Heart::hStringID(obj->nonroad(i).c_str()));
    }
}

hBool TileTextureSet::serialiseObject(projectZ::proto::projectZTileSet* obj, const Heart::hSerialisedEntitiesParameters& params) const {
    hChar guid_buffer[64];
    if (tileSet) {
        Heart::hUUID::toString(tileSet->getOwner()->getEntityID(), guid_buffer, hStaticArraySize(guid_buffer));
        obj->set_dynamictilesetid(guid_buffer);
    }
    for (const auto& i : northRoadResources) {
        obj->add_northroad(Heart::hResourceManager::getResourceID(i.resource).c_str());
    }
    for (const auto& i : eastRoadResources) {
        obj->add_eastroad(Heart::hResourceManager::getResourceID(i.resource).c_str());
    }
    for (const auto& i : southRoadResources) {
        obj->add_southroad(Heart::hResourceManager::getResourceID(i.resource).c_str());
    }
    for (const auto& i : westRoadResources) {
        obj->add_westroad(Heart::hResourceManager::getResourceID(i.resource).c_str());
    }
    for (const auto& i : intersectionRoadResources) {
        obj->add_intersectionroad(Heart::hResourceManager::getResourceID(i.resource).c_str());
    }
    for (const auto& i : nonRoadResources) {
        obj->add_nonroad(Heart::hResourceManager::getResourceID(i.resource).c_str());
    }
    return hTrue;
}

hBool TileTextureSet::linkObject() {
    if (!linkingInfo) return hTrue;
    hBool linked = true;
    if (!tileSet) {
        if (auto* entity = Heart::hEntityFactory::findEntity(linkingInfo->dynamicTextureSet)) {
            tileSet = entity->getComponent<Heart::hDynamicTileSet2D>();
        }
        linked &= !!tileSet;
    }
    for (hSize_t i=0, n=linkingInfo->northRoadResourceIDs.size(); i<n; ++i) {
        northRoadResources[i].resource = Heart::hResourceManager::weakResource<Heart::hTextureResource>(linkingInfo->northRoadResourceIDs[i]);
        linked &= !!northRoadResources[i].resource;
    }
    for (hSize_t i = 0, n = linkingInfo->eastRoadResourceIDs.size(); i < n; ++i) {
        eastRoadResources[i].resource = Heart::hResourceManager::weakResource<Heart::hTextureResource>(linkingInfo->eastRoadResourceIDs[i]);
        linked &= !!eastRoadResources[i].resource;
    }
    for (hSize_t i = 0, n = linkingInfo->southRoadResourceIDs.size(); i < n; ++i) {
        southRoadResources[i].resource = Heart::hResourceManager::weakResource<Heart::hTextureResource>(linkingInfo->southRoadResourceIDs[i]);
        linked &= !!southRoadResources[i].resource;
    }
    for (hSize_t i = 0, n = linkingInfo->westRoadResourceIDs.size(); i < n; ++i) {
        westRoadResources[i].resource = Heart::hResourceManager::weakResource<Heart::hTextureResource>(linkingInfo->westRoadResourceIDs[i]);
        linked &= !!westRoadResources[i].resource;
    }
    for (hSize_t i = 0, n = linkingInfo->intersectionRoadResourceIDs.size(); i < n; ++i) {
        intersectionRoadResources[i].resource = Heart::hResourceManager::weakResource<Heart::hTextureResource>(linkingInfo->intersectionRoadResourceIDs[i]);
        linked &= !!intersectionRoadResources[i].resource;
    }
    for (hSize_t i = 0, n = linkingInfo->nonRoadResourceIDs.size(); i < n; ++i) {
        nonRoadResources[i].resource = Heart::hResourceManager::weakResource<Heart::hTextureResource>(linkingInfo->nonRoadResourceIDs[i]);
        linked &= !!nonRoadResources[i].resource;
    }
    if (linked) {
        for (auto& i : northRoadResources) i.handle = tileSet->addTextureToTileSet(i.resource);
        for (auto& i : eastRoadResources) i.handle = tileSet->addTextureToTileSet(i.resource);
        for (auto& i : southRoadResources) i.handle = tileSet->addTextureToTileSet(i.resource);
        for (auto& i : westRoadResources) i.handle = tileSet->addTextureToTileSet(i.resource);
        for (auto& i : intersectionRoadResources) i.handle = tileSet->addTextureToTileSet(i.resource);
        for (auto& i : nonRoadResources) i.handle = tileSet->addTextureToTileSet(i.resource);
        linkingInfo.reset();
    }
    return !linkingInfo.get();
}
