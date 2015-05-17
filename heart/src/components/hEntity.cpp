/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "components/hEntity.h"
#include "components/hEntityFactory.h"

namespace Heart
{
    hRegisterObjectType(EntityDef, Heart::hEntityDef, Heart::proto::EntityDef);


    hEntityDef::~hEntityDef() {

    }

    hBool hEntityDef::serialiseObject(Heart::proto::EntityDef* obj) const {
        return hTrue;
    }

    hBool hEntityDef::deserialiseObject(Heart::proto::EntityDef* obj) {
        definition.CopyFrom(*obj);
        name = hStringID(definition.entryname().c_str());
        componentDefs.reserve(definition.components_size());
        for (auto i=0, n=definition.components_size(); i < n; ++i) {
            hComponentDef comp_def;
            comp_def.typeDefintion = hObjectFactory::getObjectDefinitionFromSerialiserName(definition.components(i).type_name().c_str());
            if (comp_def.typeDefintion) {
                comp_def.marshall = comp_def.typeDefintion->constructMarshall_();
                comp_def.marshall->ParseFromString(definition.components(i).messagedata());
                componentDefs.push_back(std::move(comp_def));
            }
        }
        hEntityFactory::registerEntityDefinition(name, this);
        return hTrue;
    }

    hBool hEntityDef::linkObject() {
        return hTrue;
    }
}