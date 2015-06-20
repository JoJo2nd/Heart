/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "base/hStringID.h"

namespace Heart {

class hEntity;
class hEntityDef;
class hEntityContext;
class hEntityComponent;
class hEntityComponentHandle;
struct hObjectDefinition;

namespace hEntityFactory {

struct hEntityCreateDesc {
    hUuid_t entityId;
    const char* entityDefinition;
};

typedef hEntityComponent* (*hComponentObjectConstructProc)(hEntityComponentHandle* handle_address);
typedef void (*hComponentObjectDestructProc)(hEntityComponent* ptr);
typedef hInt (*hComponentObjectCompactProc)();

struct hComponentMgt {
    const hObjectDefinition* object_def;
    hComponentObjectConstructProc construct;
    hComponentObjectDestructProc destruct;
    hComponentObjectCompactProc compact;
};

void registerEntityDefinition(hStringID definition_name, Heart::hEntityDef* entity_def);
void unregisterEntityDefinition(hStringID definition_name);
void registerComponentManagement(const hComponentMgt& comp_mgt);
void unregisterComponentManagement(const hObjectDefinition* object_def);
Heart::hEntityDef* getEntityDefinition(hStringID definition_name);
void unregisterEntityDefinition(hStringID definition_name);
hEntityContext* createEntityContext(const hChar* context_name, const hEntityCreateDesc* entity_defs, hSize_t entity_def_count);
void destroyEntityContext(hEntityContext* ctx);
hUuid_t createEntity(hEntityContext* context, hUuid_t id, const Heart::hEntityDef* entity_def);
void destroyEntity(hUuid_t entity_id);
hEntity* findEntity(hUuid_t entity_id);


}
}
