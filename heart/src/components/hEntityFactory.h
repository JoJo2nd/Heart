/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "base/hStringID.h"
#include "components/hObjectFactory.h"
#include "engine_state.pb.h"
#include <functional>

namespace Heart {

class hEntity;
class hEntityDef;
class hEntityContext;
class hEntityComponent;
class hEntityComponentHandle;
struct hObjectDefinition;
struct hComponentDefinition;

struct hSerialisedEntitiesParameters {
    hBool includeTransientEntites = false; // mainly for hot reload or debugging.
    Heart::proto::EngineState engineState;
};

namespace hEntityFactory {

struct hEntityCreateDesc {
    hUuid_t entityId;
    const char* entityDefinition;
};

typedef hEntityComponent* (*hComponentObjectConstructProc)();
typedef void (*hComponentObjectDestructProc)(hEntityComponent* ptr);
typedef hInt (*hComponentObjectCompactProc)();

struct hComponentMgt {
    const hObjectDefinition* object_def;
    std::function<hEntityComponent* (hEntity* owner_entity, hObjectMarshall* marshall)> construct;
    std::function<void (hEntityComponent*)> destruct;
};

void registerComponentManagement(const hComponentMgt& comp_mgt);
hEntity* createEntity(hUuid_t id, hComponentDefinition* compents, hSize_t component_def_count);
void destroyEntity(hUuid_t entity_id);
void destroyEntity(hEntity* entity);
hEntity* findEntity(hUuid_t entity_id);
void serialiseEntities(hSerialisedEntitiesParameters* in_out_params);
void deserialiseEntities(const hSerialisedEntitiesParameters& in_params);
void destroyAllEntities();

}
}
