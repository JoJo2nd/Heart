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
class hEntityContext {

	static const hUint s_entityBlockDefaultSize = 8;

    struct hEntityNode {
        hEntityNode() 
            : lnext(this)
            , lprev(this)
        {}


        hEntityNode* lnext, *lprev;
        hEntity it;
    };

	struct hEntityBlock {
		hUint entityCount;
		hUint inUse;
		hEntityBlock* lnext, *lprev;
	};

	void allocFreeListNodes() {
		auto* new_block = (hEntityBlock*)hMalloc(sizeof(hEntityBlock)+(sizeof(hEntityNode)*s_entityBlockDefaultSize));
		new (new_block) hEntityBlock();
		hEntityNode* freelist_start, *freelist_ptr, *freelist_end, *this_freelist = nullptr;
		freelist_start = freelist_ptr = (hEntityNode*)(new_block+1);
		freelist_end = freelist_start+s_entityBlockDefaultSize;
		for (; freelist_ptr < freelist_end; ++freelist_ptr) {
			new (freelist_ptr) hEntityNode();
			if (this_freelist) {
				hCircularLinkedList::insertBefore(freelist_ptr, this_freelist);
			} else {
				this_freelist = freelist_ptr;
			}
		}
        hcAssert(!freeList);
		freeList = this_freelist;
        /* else {
			for (auto* i=freeList; i->lnext != freeList; i=i->lnext) {
				if ((hUintptr_t)i > (hUintptr_t)this_freelist) {
					this_freelist->lprev->lnext = i->lnext;
					i->lnext->lprev = this_freelist->lprev;
					i->lnext = this_freelist;
					this_freelist->lprev = i;
					break;
				}
			}
		}*/
	}

public:
    hEntityContext()
        : entityBlocks(nullptr) 
        , freeList(nullptr) {
    }

    hEntity* addEntity(const hEntity& rhs) {
		if (!freeList) {
			allocFreeListNodes();
		}
		auto* new_node = freeList;
		hcAssert(new_node);
        freeList = freeList->lnext == freeList->lprev ? nullptr : freeList->lnext;
		hCircularLinkedList::remove(new_node);
		new_node->it = std::move(rhs);
		return &new_node->it;
	}
    void removeEntity(hEntity* rhs) {
		hEntityNode* node_ptr = (hEntityNode*)((hUintptr_t)rhs-(hOffsetOf(hEntityNode, it)));
		if (!freeList) {
			freeList = node_ptr;
			return;
		}

		for (auto* i = freeList; i->lnext != freeList; i = i->lnext) {
			if ((hUintptr_t)i > (hUintptr_t)node_ptr) {
				hCircularLinkedList::insertBefore(node_ptr, i);
				if (i == freeList) {
					freeList = node_ptr;
				}
				return;
			}
		}
		// Bigger than anything in the list?
		hCircularLinkedList::insertAfter(node_ptr, freeList->lprev);
	}

    hStringID contextName;
    std::vector<hEntityBlock> entities;
	hEntityBlock* entityBlocks;
    hEntityNode* freeList;
    std::vector<const hObjectDefinition*> knownTypes; // never shrinks
};

static struct hEntityContextManager {

    typedef std::unordered_map<hStringID, hEntityDef*> hEntityDefHashTable;
    typedef hEntityDefHashTable::value_type hEntityDefEntry;
    typedef std::unordered_map<hUuid_t, hEntity*> hEntityHashTable; //!!JM TODO: Make this a handle based system?
    typedef hEntityHashTable::value_type hEntityEntry;
    typedef std::unordered_map<const hObjectDefinition*, hEntityFactory::hComponentMgt> hComponentMgtHashTable;
    typedef hComponentMgtHashTable::value_type hComponentMgtEntry;

    hEntityDefHashTable entityDefTable;
    hEntityHashTable entityTable;
    std::vector<hEntityContext> contexts;
    hComponentMgtHashTable componentMgt;
} g_entityContextManager;

namespace hEntityFactory {

void registerEntityDefinition(hStringID definition_name, Heart::hEntityDef* entity_def) {
    hcAssertMsg(g_entityContextManager.entityDefTable.find(definition_name) == g_entityContextManager.entityDefTable.end(), 
        "Entity definition '%s' registered more than once", definition_name.c_str());
    g_entityContextManager.entityDefTable.insert(hEntityContextManager::hEntityDefEntry(definition_name, entity_def));
}

void registerComponentManagement(const hComponentMgt& comp_mgt) {
    hcAssertMsg(g_entityContextManager.componentMgt.find(comp_mgt.object_def) == g_entityContextManager.componentMgt.end(),
        "Component Management for '%s' registered more than once", comp_mgt.object_def->objectName_.c_str());
    g_entityContextManager.componentMgt.insert(hEntityContextManager::hComponentMgtEntry(comp_mgt.object_def, comp_mgt));
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
//         Heart::hEntity new_entity;
//         new_entity.entityId = entity_defs[i].entityId;
//         new_entity.entityComponent.reserve(etd->getComponentDefinitionCount());
//         for (hSize_t ci=0, cn=etd->getComponentDefinitionCount(); ci < cn; ++ci) {
//             auto comp_def = etd->getComponentDefinition(ci);
//             hcAssertMsg(comp_def.typeDefintion->costructComponent, "Type '%s' is not a component type", comp_def.typeDefintion->objectName_.c_str());
//             hEntityComponent* obj = comp_def.typeDefintion->costructComponent(nullptr);
//             comp_def.typeDefintion->deserialise_(obj, comp_def.marshall);
//             new_entity.entityComponent.push_back(obj);
//         }
//         ctx->entities.push_back(std::move(new_entity));
    }

    //for (auto& i : ctx->entities) {
    //    g_entityContextManager.entityTable.insert(hEntityContextManager::hEntityEntry(i.entityId, &i));
    //}
    //g_entityContextManager.contexts.push_back(ctx);
    return ctx;
}
void destroyEntityContext(hEntityContext* ctx) {
    delete ctx;
}

hUuid_t createEntity(hEntityContext* context, hUuid_t id, const Heart::hEntityDef* entity_def) {
    hcAssertMsg(context && entity_def && !hUUID::isNull(id), "Invalid args to %s", __FUNCTION__);
    Heart::hEntity new_entity;
    new_entity.entityId = id;
    new_entity.entityComponent.resize(entity_def->getComponentDefinitionCount());
    for (hSize_t ci = 0, cn = entity_def->getComponentDefinitionCount(); ci < cn; ++ci) {
        auto comp_def = entity_def->getComponentDefinition(ci);
        auto comp_mgt = g_entityContextManager.componentMgt.find(comp_def.typeDefintion);
        hcAssertMsg(comp_mgt != g_entityContextManager.componentMgt.end(), "Type '%s' is not a component type", comp_def.typeDefintion->objectName_.c_str());
        hEntityComponent* obj = comp_mgt->second.construct(&new_entity.entityComponent[ci]);
        comp_def.typeDefintion->deserialise_(obj, comp_def.marshall);
        new_entity.entityComponent[ci].update(obj);
    }

    context->addEntity(new_entity);
    return id;
}

void destroyEntity(hUuid_t entity_id) {
    auto* entity = findEntity(entity_id);
    hcAssert(entity);

    for (auto& i : entity->entityComponent) {
        if (!i.isValid()) {
            continue;
        }
        auto* mgt = i.getComponentMgt();
        mgt->destruct(i.getBase());
        i.update(nullptr);
    }
    auto* ctx = entity->owningCtx;
    ctx->removeEntity(entity);
}

hEntity* findEntity(hUuid_t entity_id) {
    return nullptr;
}


}
}