/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "zombieComponent.h"
#include "core/hResourceManager.h"
#include "render/hTextureResource.h"
#include "2d/hSprite2D.h"

hRegisterObjectType(ZombieComponent, ZombieComponent, projectZ::proto::Zombie);

ZombieComponent::ZombieComponent(projectZ::proto::Zombie* obj) {
    type = obj->type();
    if (obj->has_textureresourceid()) textureResourceID = Heart::hStringID(obj->textureresourceid().c_str());
    if (obj->has_dynamictileset()) dynamicTileSetID = Heart::hUUID::fromString(obj->dynamictileset().c_str(), obj->dynamictileset().size());
}

hBool ZombieComponent::serialiseObject(projectZ::proto::Zombie* obj) const {
    obj->set_type(type);
    obj->set_textureresourceid(textureResourceID.c_str());
    return hTrue;
}

hBool ZombieComponent::linkObject() {
    hBool linked = true;
    if (!texture && !textureResourceID.is_default()) {
        texture = Heart::hResourceManager::weakResource<Heart::hTextureResource>(textureResourceID);
        linked &= !!texture;
    }
    linked &= !!texture;
    if (texture && !tileSet) {
        if (auto* sprite = getOwner()->getComponent<Heart::hSprite2D>()) {
            /*
            if (auto* entity = Heart::hEntityFactory::findEntity(dynamicTileSetID)) {
                if (tileSet = entity->getComponent<Heart::hDynamicTileSet2D>()) {
                    tile = tileSet->addTextureToTileSet(texture);
                    sprite->setTileSet(tileSet);
                    sprite->setTileHandle(tile);
                }
            }
            */
        }
    } 
    linked &= !!tileSet;
    return linked;
}

hBool ZombieComponent::registerComponent()
{
    Heart::hEntityFactory::hComponentMgt c = {
        Heart::hObjectFactory::getObjectDefinition(ZombieComponent::getTypeNameStatic()),
        [](Heart::hEntity* owner, Heart::hObjectMarshall* marshall) -> Heart::hEntityComponent* {
            auto* real_marshall = static_cast<ZombieComponent::MarshallType*>(marshall);
            return new ZombieComponent(real_marshall);
        },
        [](Heart::hEntityComponent* ptr) { delete ptr; }
    };
    Heart::hEntityFactory::registerComponentManagement(c);
    return ZombieComponent::auto_object_registered;
}