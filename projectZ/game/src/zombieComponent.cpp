/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "zombieComponent.h"
#include "core\hResourceManager.h"
#include "render\hTextureResource.h"

hRegisterObjectType(ZombieComponent, ZombieComponent, projectZ::proto::Zombie);

hBool ZombieComponent::serialiseObject(projectZ::proto::Zombie* obj) const {
    obj->set_type(type);
    obj->set_textureresourceid(textureResourceID.c_str());
    return hTrue;
}
hBool ZombieComponent::deserialiseObject(projectZ::proto::Zombie* obj) {
    if (obj->has_type()) type = obj->type();
    if (obj->has_textureresourceid()) textureResourceID = Heart::hStringID(obj->textureresourceid().c_str());
    return hTrue;
}
hBool ZombieComponent::linkObject() {
    hBool linked = true;
    if (!texture && !textureResourceID.is_default()) {
        texture = Heart::hResourceManager::weakResource<Heart::hTextureResource>(textureResourceID);
        linked &= !!texture;
    }
    return linked;
}

hBool ZombieComponent::registerComponent()
{
    Heart::hEntityFactory::hComponentMgt c = {
        Heart::hObjectFactory::getObjectDefinition(ZombieComponent::getTypeNameStatic()),
        [](Heart::hEntity* owner) -> Heart::hEntityComponent* { return new ZombieComponent(); },
        [](Heart::hEntityComponent* ptr) { delete ptr; }
    };
    Heart::hEntityFactory::registerComponentManagement(c);
    return ZombieComponent::auto_object_registered;
}