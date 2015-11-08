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
#if 0
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
#endif

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
        obj = comp_mgt->second.construct(new_entity);
        obj->componentDestruct = comp_mgt->second.destruct;
        comp_def.typeDefintion->deserialise_(obj, comp_def.marshall);
        new_entity->entityComponents[ci].typeID = comp_def.typeDefintion->runtimeTypeID;
        new_entity->entityComponents[ci].typeDef = comp_def.typeDefintion;
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

hEntity* findEntity(hUuid_t entity_id) {
    auto it = g_entityContextManager.entityTable.find(entity_id);
    return it != g_entityContextManager.entityTable.end() ? it->second : nullptr;
}


}
}