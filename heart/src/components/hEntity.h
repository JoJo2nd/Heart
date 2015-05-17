/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "components/hObjectFactory.h"
#include "entity_def.pb.h"

namespace Heart {
    class hEntityDef {
    public:
        hObjectType(Heart::hEntityDef, Heart::proto::EntityDef);

        ~hEntityDef();

        struct hComponentDef {
            const hObjectDefinition* typeDefintion;
            hObjectMarshall* marshall;
        };

        hSize_t getComponentDefinitionCount() const {
            return componentDefs.size();
        }
        hComponentDef getComponentDefinition(hSize_t i) {
            return componentDefs[i];
        }
    private:

        hStringID name;
        Heart::proto::EntityDef definition;
        std::vector<hComponentDef> componentDefs;
    };

    class hEntityComponent {
    public:
        virtual ~hEntityComponent() {}
    };

    class hEntity {
    public:
    //private:

        hUuid_t entityId;
        std::vector<hEntityComponent*> entityComponent;
    };
}

