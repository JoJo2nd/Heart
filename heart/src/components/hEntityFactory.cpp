/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "components/hEntityFactory.h"
#include "base/hUUID.h"
#include "base/hMemory.h"
#include "base/hMemoryUtil.h"
#include "base/hLinkedList.h"
#include "components/hEntity.h"
#include <unordered_map>
#include <unordered_set>
#include <memory>



namespace Heart {
static struct hEntityContextManager {
    typedef std::unordered_map<hUuid_t, hEntity*> hEntityHashTable; //!!JM TODO: Make this a handle based system?
    typedef hEntityHashTable::value_type hEntityEntry;
    typedef std::unordered_map<const hObjectDefinition*, hEntityFactory::hComponentMgt> hComponentMgtHashTable;
    typedef hComponentMgtHashTable::value_type hComponentMgtEntry;

    hEntityHashTable entityTable;
    hComponentMgtHashTable componentMgt;
} g_entityContextManager;

namespace hEntityFactory {

void registerComponentManagement(const hComponentMgt& comp_mgt) {
    hcAssertMsg(g_entityContextManager.componentMgt.find(comp_mgt.object_def) == g_entityContextManager.componentMgt.end(),
        "Component Management for '%s' registered more than once", comp_mgt.object_def->objectName_.c_str());
    g_entityContextManager.componentMgt.insert(hEntityContextManager::hComponentMgtEntry(comp_mgt.object_def, comp_mgt));
}

hEntity* createEntity(hUuid_t id, hComponentDefinition* compents, hSize_t component_def_count) {
    hcAssertMsg(compents && !hUUID::isNull(id), "Invalid args to %s", __FUNCTION__);
    Heart::hEntity* new_entity = new Heart::hEntity(); // TODO!!JM Freelist this!
    new_entity->entityId = id;
    new_entity->entityComponents.resize(component_def_count);
    for (hSize_t ci = 0, cn = component_def_count; ci < cn; ++ci) {
        const auto& comp_def = compents[ci];
        auto comp_mgt = g_entityContextManager.componentMgt.find(comp_def.typeDefintion);
        hEntityComponent* obj = nullptr;
        hcAssertMsg(comp_mgt != g_entityContextManager.componentMgt.end(), "Type '%s' is not a component type", comp_def.typeDefintion->objectName_.c_str());
        obj = comp_mgt->second.construct(new_entity, comp_def.marshall);
        obj->owner = new_entity;
        obj->componentDestruct = comp_mgt->second.destruct;
        new_entity->entityComponents[ci].typeID = comp_def.typeDefintion->runtimeTypeID;
        new_entity->entityComponents[ci].typeDef = comp_def.typeDefintion;
        new_entity->entityComponents[ci].id = comp_def.id;
        new_entity->entityComponents[ci].ptr = obj;
    }

    g_entityContextManager.entityTable[id] = new_entity;//!
    return new_entity;
}

void destroyEntity(hUuid_t entity_id) {
    auto* entity = findEntity(entity_id);
    hcAssert(entity);

    for (auto& i : entity->entityComponents) {
        i.ptr->componentDestruct(i.ptr);
    }
    g_entityContextManager.entityTable.erase(g_entityContextManager.entityTable.find(entity_id));
    delete entity;
}

void destroyEntity(hEntity* entity) {
    hcAssert(entity);
    hUuid_t entity_id = entity->getEntityID();

    for (auto& i : entity->entityComponents) {
        i.ptr->componentDestruct(i.ptr);
    }
    g_entityContextManager.entityTable.erase(g_entityContextManager.entityTable.find(entity_id));
    delete entity;
}

hEntity* findEntity(hUuid_t entity_id) {
    auto it = g_entityContextManager.entityTable.find(entity_id);
    return it != g_entityContextManager.entityTable.end() ? it->second : nullptr;
}

void serialiseEntities(hSerialisedEntitiesParameters* in_out_params) {
    hcAssert(in_out_params);
    auto& state = in_out_params->engineState;
    hChar entity_guid_str_buff[64];
    for (const auto& i : g_entityContextManager.entityTable) {
        hUUID::toString(i.first, entity_guid_str_buff, hStaticArraySize(entity_guid_str_buff));
        state.add_aliveentityid(entity_guid_str_buff);
        auto* entity_def = state.add_aliveentities();
        auto* entity = i.second;
        state.set_maxentitycomponentcount(hMax(state.maxentitycomponentcount(), i.second->getComponentCount()));
        if (in_out_params->includeTransientEntites || !entity->getTransient())
            entity->serialise(entity_def, entity_guid_str_buff, *in_out_params);
    }

}

void deserialiseEntities(const hSerialisedEntitiesParameters& in_params) {
    std::vector<hComponentDefinition> components;
    components.reserve(in_params.engineState.maxentitycomponentcount());
    for (hInt i = 0, n = in_params.engineState.aliveentities_size(); i < n; ++i) {
        auto& entity = in_params.engineState.aliveentities(i);
        hUuid_t guid = hUUID::fromString(entity.objectguid().c_str(), entity.objectguid().length());
        hSize_t totalComponents = entity.components_size();
        components.clear();
        components.reserve(totalComponents);
        for (hInt ci = 0, cn = entity.components_size(); ci < cn; ++ci) {
            hComponentDefinition comp_def;
            comp_def.typeDefintion = hObjectFactory::getObjectDefinitionFromSerialiserName(entity.components(ci).type_name().c_str());
            comp_def.id = entity.components(ci).componentid();
            if (comp_def.typeDefintion) {
                comp_def.marshall = comp_def.typeDefintion->constructMarshall_();
                comp_def.marshall->ParseFromString(entity.components(ci).messagedata());
                components.push_back(std::move(comp_def));
            }
        }

        auto* new_entity = hEntityFactory::createEntity(guid, components.data(), components.size());
        new_entity->setTransitent(entity.has_transient() ? entity.transient() : false);
    }
}

void destroyAllEntities() {
    for (const auto& i : g_entityContextManager.entityTable) {
        destroyEntity(i.second);
    }
}

}

void hEntity::serialise(proto::EntityDefinition* obj, const char* entity_guid_str, const hSerialisedEntitiesParameters& params) const {
    obj->set_transient(transient);
    obj->set_friendlyname(friendlyName.c_str());
    obj->set_objectguid(entity_guid_str);
    for (const auto& i : entityComponents) {
        auto* msg_cntr = obj->add_components();
        hObjectMarshall* marshall = i.typeDef->constructMarshall_(); // We could pool these, make them cheaper on memory.
        msg_cntr->set_type_name(i.typeDef->serialiserName_.c_str());
        msg_cntr->set_componentid(i.id);
        i.typeDef->serialise_(i.ptr, marshall, params);
    }
}

}