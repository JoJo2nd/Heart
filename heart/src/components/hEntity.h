/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "components/hObjectFactory.h"
#include "components/hEntityFactory.h"
#include "entity_def.pb.h"
#include <functional>

namespace Heart {
    class hEntity;
    class hEntityContext;

    struct hComponentDefinition {
        const hObjectDefinition* typeDefintion = nullptr;
        hObjectMarshall* marshall = nullptr;
        hUint32 id = 0; // component ID within an entity
    };

    class hEntityComponent {
        hEntity* owner = nullptr;

        friend hEntity* hEntityFactory::createEntity(hUuid_t, hComponentDefinition*, hSize_t);
        friend void hEntityFactory::destroyEntity(hUuid_t);
    public:
        virtual ~hEntityComponent() {}
    
        hEntity* getOwner() const { return owner; }
        virtual void onOwningLevelLoadComplete() {};
        std::function<void (hEntityComponent*)> componentDestruct;
    };

    class hEntity {
    public:
        struct Component {
            hUintptr_t typeID = 0;
            const hObjectDefinition* typeDef = nullptr;
            hEntityComponent* ptr = nullptr;
            hUint32 id = 0;
            hBool linked = false;
        };

        template < typename t_ty >
        t_ty* getComponent() {
            for (const auto& i : entityComponents) {
                if (i.typeID == t_ty::getRuntimeTypeID() && i.linked)
                    return static_cast<t_ty*>(i.ptr);
            }
            return nullptr;
        }
        template < typename t_ty >
        t_ty* getComponent(hUint32 in_id) {
            for (const auto& i : entityComponents) {
                if (i.typeID == t_ty::getRuntimeTypeID() && i.id == in_id && i.linked)
                    return static_cast<t_ty*>(i.ptr);
            }
            return nullptr;
        }
        hUint getComponentCount() const { return (hUint)entityComponents.size(); } 
        hUuid_t getEntityID() const { return entityId; }
        void setTransitent(hBool val) { transient = val; }
        hBool getTransient() const { return transient; }
        const std::vector<Component>& getComponents() const { return entityComponents; }
#if HEART_DEBUG_INFO
        const hChar* getFriendlyName() const { return friendlyName.c_str(); }
        void setFriendlyName(const hChar* name) { friendlyName = hStringID(name); }
#endif
        void serialise(proto::EntityDefinition* obj, const char* entity_guid_str, const hSerialisedEntitiesParameters& params) const;

    private:
        friend hEntity* hEntityFactory::createEntity(hUuid_t, hComponentDefinition*, hSize_t);
        friend void hEntityFactory::destroyEntity(hUuid_t);
        friend void hEntityFactory::destroyEntity(hEntity*);
        friend class hLevel;

        hUuid_t entityId;
        hBool transient = false;
        std::vector<Component> entityComponents;
        hStringID friendlyName;
    };

}
