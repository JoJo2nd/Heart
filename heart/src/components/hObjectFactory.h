/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "base/hTypes.h"
#include "base/hStringID.h"
#include "core/hProtobuf.h"

namespace Heart {
    typedef ::google::protobuf::MessageLite hObjectMarshall;
    class hEntityComponent;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    typedef void* (*hObjectConstructProc)();
    typedef hEntityComponent* (*hComponentObjectConstructProc)();
    typedef void (*hObjectDestroyProc)(void*);
    typedef hObjectMarshall* (*hObjectCreateSerialiserProc)();
    typedef hBool (*hObjectSerialiseProc)(void*, hObjectMarshall*);
    typedef hBool (*hObjectDeserialiseProc)(void*, hObjectMarshall*);
    typedef hBool (*hObjectLinkProc)(void*);

    struct hObjectDefinition 
    {
        hStringID                       objectName_;
        hObjectConstructProc            construct_;
        hComponentObjectConstructProc   costructComponent;
        hObjectDestroyProc              destroy_;
        hObjectCreateSerialiserProc     constructMarshall_;
        hObjectSerialiseProc            serialise_;
        hObjectDeserialiseProc          deserialise_;
        hObjectLinkProc                 link_;
        hStringID                       serialiserName_;
        std::vector<hStringID>          baseTypes_;
    };

#define hObjectType(name, serialiser_type) \
    static hBool auto_object_registered;\
    static Heart::hStringID getTypeNameStatic() { \
        static Heart::hStringID typeName(#name); \
        return typeName; \
    } \
    virtual Heart::hStringID getTypeName() const { \
        return getTypeNameStatic(); \
    } \
    hBool serialiseObject(serialiser_type*) const; \
    hBool deserialiseObject(serialiser_type*); \
    hBool linkObject()

#define hObjectBaseType(x) hObjectBaseTypeInner(#x)

#define hObjectBaseTypeInner(x) (##x)

#define hRegisterObjectType(name, type, serialiser_type, ...) \
    static void* autogen_construct_##name () { return new type; } \
    static void autogen_destroy_##name(void* d) { delete ((type*)d); } \
    static Heart::hObjectMarshall* autogen_create_serialiser_##name () { return new serialiser_type; } \
    static hBool autogen_serialise_##name(void* type_ptr_raw, Heart::hObjectMarshall* msg_raw) { \
        type* type_ptr = reinterpret_cast<type*>(type_ptr_raw); \
        serialiser_type* msg = static_cast<serialiser_type*>(msg_raw); \
        return type_ptr->serialiseObject(msg); \
    } \
    static hBool autogen_deserialise_##name(void* type_ptr_raw, Heart::hObjectMarshall* msg_raw) { \
        type* type_ptr = reinterpret_cast<type*>(type_ptr_raw); \
        serialiser_type* msg = static_cast<serialiser_type*>(msg_raw); \
        return type_ptr->deserialiseObject(msg); \
    } \
    static hBool autogen_link_##name(void* type_ptr_raw) {\
        type* type_ptr = reinterpret_cast<type*>(type_ptr_raw); \
        return type_ptr->linkObject(); \
    }\
    static Heart::hObjectDefinition autogen_entity_definition_##name = { \
        type::getTypeNameStatic(), \
        autogen_construct_##name, \
        nullptr, \
        autogen_destroy_##name, \
        autogen_create_serialiser_##name, \
        autogen_serialise_##name, \
        autogen_deserialise_##name, \
        autogen_link_##name, \
    };\
    hBool type::auto_object_registered = Heart::hObjectFactory::objectFactoryRegistar(&autogen_entity_definition_##name, #serialiser_type, ##__VA_ARGS__, nullptr)

#define hRegisterComponentObjectType(name, type, serialiser_type, ...) \
    static void* autogen_construct_##name () { return new type; } \
    static Heart::hEntityComponent* autogen_construct_component_##name () { return new type; } \
    static void autogen_destroy_##name(void* d) { delete ((type*)d); } \
    static Heart::hObjectMarshall* autogen_create_serialiser_##name () { return new serialiser_type; } \
    static hBool autogen_serialise_##name(void* type_ptr_raw, Heart::hObjectMarshall* msg_raw) { \
        type* type_ptr = reinterpret_cast<type*>(type_ptr_raw); \
        serialiser_type* msg = static_cast<serialiser_type*>(msg_raw); \
        return type_ptr->serialiseObject(msg); \
        } \
    static hBool autogen_deserialise_##name(void* type_ptr_raw, Heart::hObjectMarshall* msg_raw) { \
        type* type_ptr = reinterpret_cast<type*>(type_ptr_raw); \
        serialiser_type* msg = static_cast<serialiser_type*>(msg_raw); \
        return type_ptr->deserialiseObject(msg); \
        } \
    static hBool autogen_link_##name(void* type_ptr_raw) {\
        type* type_ptr = reinterpret_cast<type*>(type_ptr_raw); \
        return type_ptr->linkObject(); \
        }\
    static Heart::hObjectDefinition autogen_entity_definition_##name = { \
        type::getTypeNameStatic(), \
        autogen_construct_##name, \
        autogen_construct_component_##name, \
        autogen_destroy_##name, \
        autogen_create_serialiser_##name, \
        autogen_serialise_##name, \
        autogen_deserialise_##name, \
        autogen_link_##name, \
        };\
    hBool type::auto_object_registered = Heart::hObjectFactory::objectFactoryRegistar(&autogen_entity_definition_##name, #serialiser_type, ##__VA_ARGS__, nullptr)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
    
namespace hObjectFactory {
    const hObjectDefinition*        getObjectDefinition(hStringID name);
    const hObjectDefinition*        getObjectDefinitionFromSerialiserName(const hChar* name);
    void*                           createObject(hStringID name);
    hObjectMarshall*                createObjectMarshallFromTypeName(hStringID name);
    void*                           deserialiseObject(Heart::proto::MessageContainer* msg_container, hStringID* out_type_name);
    hBool                           objectFactoryRegistar(hObjectDefinition*, const char*, ...);
    template< typename t_ty >
    hFORCEINLINE hObjectMarshall*   createObjectMarshallFromType(t_ty* ptr) {
        return createObjectMarshallFromType(ptr->getTypeName());
    }
    hBool                           canUpcastTo(hStringID type1, hStringID type2);
    template< typename t_type1, typename t_type2 >
    hBool                           isA(const t_type1* t1, const t_type2* t2) {
        return t1->getTypeName() == t2->getTypeName();
    }
}

}
