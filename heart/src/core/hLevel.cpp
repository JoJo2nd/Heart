/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "core/hLevel.h"
#include "base/hUUID.h"
#include "components/hEntity.h"
#include "components/hEntityFactory.h"

namespace Heart {
hRegisterObjectType(LevelDefinition, Heart::hLevel, Heart::proto::LevelDefinition);

hBool hLevel::serialiseObject(Heart::proto::LevelDefinition* obj) const {
    return hTrue;
}

hBool hLevel::deserialiseObject(Heart::proto::LevelDefinition* obj) {
    std::vector<hComponentDefinition> components;
    for (hInt i=0, n=obj->entities_size(); i<n; ++i) {
        auto& entity = obj->entities(i);
        hUuid_t guid = hUUID::fromString(entity.objectguid().c_str(), entity.objectguid().length());
        components.clear();
        components.reserve(entity.components_size());
        for (hInt ci = 0, cn = entity.components_size(); ci<cn; ++ci) {
            //hUuid_t createEntity(hUuid_t id, hComponentDefinition* compents, hSize_t component_def_count);
            hComponentDefinition comp_def;
            comp_def.typeDefintion = hObjectFactory::getObjectDefinitionFromSerialiserName(entity.components(ci).type_name().c_str());
            if (comp_def.typeDefintion) {
                comp_def.marshall = comp_def.typeDefintion->constructMarshall_();
                comp_def.marshall->ParseFromString(entity.components(ci).messagedata());
                components.push_back(std::move(comp_def));
            }
        }
        hEntityFactory::createEntity(guid, components.data(), components.size());
    }
    return hTrue;
}

hBool hLevel::linkObject() {
    return hTrue;
}
}