/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "components/hObjectFactory.h"
#include "base/hMemory.h"
#include <unordered_map>
#include <stdarg.h>

namespace Heart
{
namespace hObjectFactory
{
namespace
{
    typedef std::unordered_map< hStringID, hObjectDefinition* >   hObjectDefinitionTable;
    typedef std::unordered_map< hStringID, hStringID >            hSerialiserToObjectTable;

    static hBool                    doneGlobalInit_ = hFalse;
    static hObjectDefinitionTable*  objectDefTable_;
    static hObjectDefinitionTable*  serialiserDefTable_;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const hObjectDefinition* getObjectDefinition(hStringID name) {
    const auto definition = objectDefTable_->find(name);
    if (definition == objectDefTable_->end()) {
        return nullptr;
    }
    return definition->second;
}

const hObjectDefinition* getObjectDefinitionFromSerialiserName(const hChar* name) {
    hObjectDefinition* obj_def = (*serialiserDefTable_)[hStringID(name)];
    return obj_def;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* createEntity(hStringID name) {
    auto definition = objectDefTable_->find(name);
    if (definition == objectDefTable_->end()) {
        return hNullptr;
    }
    void* ptr=definition->second->construct_(nullptr);
    return ptr;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hObjectMarshall* createObjectMarshallFromTypeName(hStringID name) {
    auto definition = objectDefTable_->find(name);
    if (definition == objectDefTable_->end()) {
        return hNullptr;
    }
    return definition->second->constructMarshall_();;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool objectFactoryRegistar(hObjectDefinition* definition, const char* serialiser_type_name, ...) {
    static hObjectDefinitionTable   localObjectDefTable_;
    static hObjectDefinitionTable   localSerialiserDefTable_;
    if (objectDefTable_ == nullptr) {
        objectDefTable_=&localObjectDefTable_;
    }
    if (serialiserDefTable_ == nullptr) {
        serialiserDefTable_ = &localSerialiserDefTable_;
    }
    hcAssertMsg(doneGlobalInit_ == hFalse, "Registering Type too later, this should be done before main()");
    if (doneGlobalInit_ || definition->objectName_.is_default()) {
        return hFalse;
    }
    (*objectDefTable_)[definition->objectName_] = definition;

    hString real_serialiser_type_name = serialiser_type_name;
    for (auto loc=real_serialiser_type_name.find("::"); loc != real_serialiser_type_name.npos; loc=real_serialiser_type_name.find("::")) {
        real_serialiser_type_name.replace(loc, 2, ".");
    }
    
    definition->serialiserName_ = hStringID(real_serialiser_type_name.c_str());
    (*serialiserDefTable_)[definition->serialiserName_] = definition;

    va_list marker;
    va_start(marker, serialiser_type_name);

    for(const char* base_type_name=va_arg(marker, const char*); base_type_name; base_type_name=va_arg(marker, const char*)) {
        definition->baseTypes_.push_back(hStringID(base_type_name));
    }

    va_end(marker);
    return hTrue;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* deserialiseObject(Heart::proto::MessageContainer* msg_container, hStringID* out_type_name) {
    const hObjectDefinition* obj_def = getObjectDefinitionFromSerialiserName(msg_container->type_name().c_str());
    if (!obj_def) {
        return nullptr;
    }
    hObjectMarshall* marshall = obj_def->constructMarshall_();
    if (!marshall) {
        return nullptr;
    }
    if (!marshall->ParseFromString(msg_container->messagedata())) {
        return nullptr;
    }
    void* obj = obj_def->construct_(marshall);
    delete marshall;
    marshall = nullptr;
    if (out_type_name) {
        *out_type_name = obj_def->objectName_;
    }
    return obj;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool canUpcastTo(hStringID type1, hStringID type2) {
    if (type1 == type2) {
        return hTrue;
    }
    hObjectDefinition* obj_def = (*objectDefTable_)[type1];
    if (!obj_def) {
        return hFalse;
    }
    for (auto i=obj_def->baseTypes_.cbegin(), n=obj_def->baseTypes_.cend(); i!=n; ++i) {
        if (*i == type2) {
            return hTrue;
        }
        if (canUpcastTo(*i, type2)) {
            return hTrue;
        }
    }
    return hFalse;
}
}
}
