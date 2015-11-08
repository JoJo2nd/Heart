/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "components/hObjectFactory.h"
#include "entity_def.pb.h"
#include <functional>

namespace Heart {
    class hEntityContext;
namespace hEntityFactory {
    struct hComponentMgt;
}

    struct hComponentDefinition {
        const hObjectDefinition* typeDefintion;
        hObjectMarshall* marshall;
    };

    class hEntityComponent {
    public:
        virtual ~hEntityComponent() {}
    
        std::function<void (hEntityComponent*)> componentDestruct;
    };

    class hEntity {
    public:
        struct Component {
            hUintptr_t typeID;
            const hObjectDefinition* typeDef;
            hEntityComponent* ptr;
        };

        hEntity() 
            : transient(false) {
        }
    //private: // ?? - not sure we want anyone messing with these

        hUuid_t entityId;
        hBool transient;
        std::vector<Component> entityComponents;
    };
}

