/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "components/hObjectFactory.h"
#include "entity_def.pb.h"

namespace Heart {
    class hEntityContext;
namespace hEntityFactory {
    struct hComponentMgt;
}

    struct hComponentDefinition {
        const hObjectDefinition* typeDefintion;
        hObjectMarshall* marshall;
    };

    class hEntityDef {
    public:
        hObjectType(hEntityDef, Heart::proto::EntityDef);

        ~hEntityDef();

        struct hComponentDef {
            const hObjectDefinition* typeDefintion;
            hObjectMarshall* marshall;
        };

        hSize_t getComponentDefinitionCount() const {
            return componentDefs.size();
        }
        hComponentDef getComponentDefinition(hSize_t i) const {
            return componentDefs[i];
        }
    private:

        hStringID name;
        Heart::proto::EntityDef definition;
        std::vector<hComponentDef> componentDefs;
    };

    class hEntityComponentHandle {
    public:
        hEntityComponentHandle() 
            : proc(nullptr)
            , ptr(nullptr)
        {}
        explicit hEntityComponentHandle(hEntityFactory::hComponentMgt* in_proc, hEntityComponent* in_ptr)
            : proc(in_proc)
            , ptr(in_ptr)
        {}
        template< typename t_ty >
        t_ty* get() {
            return static_cast<t_ty*>(ptr);
        }
        hEntityComponent* getBase() {
            return ptr;
        }
        bool isValid() {
            return ptr != nullptr && proc != nullptr;
        }
        void update(hEntityComponent* in_ptr) {
            ptr = in_ptr;
        }
        const hEntityFactory::hComponentMgt* getComponentMgt() const {
            return proc;
        }
    private:
        hEntityFactory::hComponentMgt* proc;
        hEntityComponent* ptr;
    };

    class hEntityComponent {
    public:
        virtual ~hEntityComponent() {}
    };

    class hEntity {
    public:
    //private: // ?? - not sure we want anyone messing with these

        hUuid_t entityId;
        hEntityContext* owningCtx;
        std::vector<hEntityComponentHandle> entityComponent;
    };
}

