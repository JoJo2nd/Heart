/********************************************************************

    filename:   hObjectFactory.cpp  
    
    Copyright (c) 23:2:2014 James Moran
    
    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.
    
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
    
    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
    
    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
    
    3. This notice may not be removed or altered from any source
    distribution.

*********************************************************************/

namespace Heart
{
namespace hObjectFactory
{
namespace
{
    typedef std::unordered_map< hStringID, hObjectDefinition*, hStringID::hasher >   hObjectDefinitionTable;
    typedef std::unordered_map< hStringID, hStringID, hStringID::hasher >            hSerialiserToObjectTable;

    static hBool                    doneGlobalInit_ = hFalse;
    static hObjectDefinitionTable   objectDefTable_;
    static hObjectDefinitionTable   serialiserDefTable_;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const hObjectDefinition* getEntityDefinition(hStringID name) {
    const auto definition = objectDefTable_.find(name);
    if (definition == objectDefTable_.end()) {
        return hNullptr;
    }
    return definition->second;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* createEntity(hStringID name) {
    auto definition = objectDefTable_.find(name);
    if (definition == objectDefTable_.end()) {
        return hNullptr;
    }
    void* ptr=definition->second->construct_();
    return ptr;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hObjectMarshall* createObjectMarshallFromTypeName(hStringID name) {
    auto definition = objectDefTable_.find(name);
    if (definition == objectDefTable_.end()) {
        return hNullptr;
    }
    return definition->second->constructMarshall_();;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool objectFactoryRegistar(hObjectDefinition* definition, const char* serialiser_type_name, ...) {
    hcAssertMsg(doneGlobalInit_ == hFalse, "Registering Type too later, this should be done before main()");
    if (doneGlobalInit_ || definition->entityName_.is_default()) {
        return hFalse;
    }
    objectDefTable_[definition->entityName_] = definition;

    hString real_serialiser_type_name = serialiser_type_name;
    for (auto loc=real_serialiser_type_name.find("::"); loc != real_serialiser_type_name.npos; loc=real_serialiser_type_name.find("::")) {
        real_serialiser_type_name.replace(loc, 2, ".");
    }
    
    definition->serialiserName_ = hStringID(real_serialiser_type_name.c_str());
    serialiserDefTable_[definition->serialiserName_] = definition;

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
    hObjectDefinition* obj_def = serialiserDefTable_[hStringID(msg_container->type_name().c_str())];
    if (!obj_def) {
        return nullptr;
    }
    void* obj = obj_def->construct_();
    hObjectMarshall* marshall = obj_def->constructMarshall_();
    if (!obj || !marshall) {
        hDELETE_SAFE(obj);
        hDELETE_SAFE(marshall);
        return nullptr;
    }
    marshall->ParseFromString(msg_container->messagedata());
    if (!obj_def->deserialise_(obj, marshall)) {
        hDELETE_SAFE(obj);
    }
    hDELETE_SAFE(marshall);
    if (out_type_name) {
        *out_type_name = obj_def->entityName_;
    }
    return obj;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool hObjectFactory::canUpcastTo(hStringID type1, hStringID type2) {
    if (type1 == type2) {
        return hTrue;
    }
    hObjectDefinition* obj_def = objectDefTable_[type1];
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
