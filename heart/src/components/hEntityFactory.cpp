/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "components/hEntityFactory.h"
#include "components/hEntity.h"
#include <unordered_map>



namespace Heart {
class hEntityContext {

public:
    hStringID contextName;
    std::vector<hEntity> entities;
};

struct hEntityContextManager {

    typedef std::unordered_map<hStringID, hEntityDef*> hEntityDefHashTable;
    typedef hEntityDefHashTable::value_type hEntityDefEntry;
    typedef std::unordered_map<hUuid_t, hEntity*> hEntityHashTable; //!!JM TODO: Make this a handle based system?
    typedef hEntityHashTable::value_type hEntityEntry;

    hEntityDefHashTable entityDefTable;
    hEntityHashTable entityTable;
    std::vector<hEntityContext> contexts;
} g_entityContextManager;

namespace hEntityFactory {

void registerEntityDefinition(hStringID definition_name, Heart::hEntityDef* entity_def) {
    hcAssertMsg(g_entityContextManager.entityDefTable.find(definition_name) == g_entityContextManager.entityDefTable.end(), 
        "Entity definition '%s' registered more than once", definition_name.c_str());
    g_entityContextManager.entityDefTable.insert(hEntityContextManager::hEntityDefEntry(definition_name, entity_def));
}

Heart::hEntityDef* getEntityDefinition(hStringID definition_name) {
    auto i = g_entityContextManager.entityDefTable.find(definition_name);
    return i == g_entityContextManager.entityDefTable.end() ? nullptr : i->second;
}

void unregisterEntityDefinition(hStringID definition_name) {
    g_entityContextManager.entityDefTable.erase(definition_name);
}
hEntityContext* createEntityContext(const char* context_name, const hEntityCreateDesc* entity_defs, hSize_t entity_def_count) {
    auto* ctx = new hEntityContext();
    ctx->contextName = hStringID(context_name);
    ctx->entities.reserve(entity_def_count);
    //!!JM!!TODO: Scope Allocator and smarter component allocation
    for (auto i=0u; i < entity_def_count; ++i) {
        auto* etd = getEntityDefinition(hStringID(entity_defs[i].entityDefinition));
        hcAssertMsg(etd, "Failed to find entity definition for '%s'", entity_defs[i].entityDefinition);
        Heart::hEntity new_entity;
        new_entity.entityId = entity_defs[i].entityId;
        new_entity.entityComponent.reserve(etd->getComponentDefinitionCount());
        for (hSize_t ci=0, cn=etd->getComponentDefinitionCount(); ci < cn; ++ci) {
            auto comp_def = etd->getComponentDefinition(ci);
            hcAssertMsg(comp_def.typeDefintion->costructComponent, "Type '%s' is not a component type", comp_def.typeDefintion->objectName_.c_str());
            hEntityComponent* obj = comp_def.typeDefintion->costructComponent();
            comp_def.typeDefintion->deserialise_(obj, comp_def.marshall);
            new_entity.entityComponent.push_back(obj);
        }
        ctx->entities.push_back(std::move(new_entity));
    }

    for (auto& i : ctx->entities) {
        g_entityContextManager.entityTable.insert(hEntityContextManager::hEntityEntry(i.entityId, &i));
    }
    //g_entityContextManager.contexts.push_back(ctx);
    return ctx;
}
void destroyEntityContext(hEntityContext* ctx) {
    delete ctx;
}
hEntity* findEntity(hUuid_t entity_id) {
    return nullptr;
}


}
}