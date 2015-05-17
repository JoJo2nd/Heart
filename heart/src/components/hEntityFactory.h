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

namespace hEntityFactory {

struct hEntityCreateDesc {
    hUuid_t entityId;
    const char* entityDefinition;
};

void registerEntityDefinition(hStringID definition_name, Heart::hEntityDef* entity_def);
Heart::hEntityDef* getEntityDefinition(hStringID definition_name);
void unregisterEntityDefinition(hStringID definition_name);
hEntityContext* createEntityContext(const hChar* context_name, const hEntityCreateDesc* entity_defs, hSize_t entity_def_count);
void destroyEntityContext(hEntityContext* ctx);
hEntity* findEntity(hUuid_t entity_id);


}
}
