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
    struct hSerialisedEntitiesParameters;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    typedef void* (*hObjectConstructProc)(Heart::hObjectMarshall* in_place);
    typedef void (*hObjectDestructProc)(void*);
    typedef void (*hObjectDestroyProc)(void*);
    typedef hObjectMarshall* (*hObjectCreateSerialiserProc)();
    typedef hBool (*hObjectSerialiseProc)(void*, hObjectMarshall*, const Heart::hSerialisedEntitiesParameters&);
    typedef hBool (*hObjectLinkProc)(void*);

    struct hObjectDefinition 
    {
        hUintptr_t                      runtimeTypeID;
        hStringID                       objectName_;
        hSize_t                         typeSize;
        hObjectConstructProc            construct_;
        hObjectDestructProc             destruct;
        hObjectDestroyProc              destroy_;
        hObjectCreateSerialiserProc     constructMarshall_;
        hObjectSerialiseProc            serialise_;
        hObjectLinkProc                 link_;
        hStringID                       serialiserName_;
        std::vector<hStringID>          baseTypes_;
    };

#define hObjectType(name, serialiser_type) \
    typedef serialiser_type MarshallType; \
    static hBool auto_object_registered;\
    static hUintptr_t getRuntimeTypeID() { \
        return (hUintptr_t)&auto_object_registered; \
    } \
    static Heart::hStringID getTypeNameStatic() { \
        static Heart::hStringID typeName(#name); \
        return typeName; \
    } \
    static void dtor(name* a) {\
        a->~name();\
    }\
    name(serialiser_type*); \
    virtual Heart::hStringID getTypeName() const { \
        return getTypeNameStatic(); \
    } \
    hBool serialiseObject(serialiser_type*, const Heart::hSerialisedEntitiesParameters&) const; \
    hBool linkObject()

#define hObjectBaseType(x) hObjectBaseTypeInner(#x)

#define hObjectBaseTypeInner(x) (##x)

#define hRegisterObjectType(name, type, serialiser_type, ...) \
    static void* autogen_construct_##name (Heart::hObjectMarshall* marshall) { \
        serialiser_type* real_marshall = static_cast<serialiser_type*>(marshall); \
        return new type(real_marshall); } \
    static void autogen_destruct_##name(void* type_ptr_raw) { type::dtor(reinterpret_cast<type*>(type_ptr_raw)); } \
    static void autogen_destroy_##name(void* d) { delete ((type*)d); } \
    static Heart::hObjectMarshall* autogen_create_serialiser_##name () { return new serialiser_type; } \
    static hBool autogen_serialise_##name(void* type_ptr_raw, Heart::hObjectMarshall* msg_raw, const Heart::hSerialisedEntitiesParameters& params) { \
        type* type_ptr = reinterpret_cast<type*>(type_ptr_raw); \
        serialiser_type* msg = static_cast<serialiser_type*>(msg_raw); \
        return type_ptr->serialiseObject(msg, params); \
    } \
    static hBool autogen_link_##name(void* type_ptr_raw) {\
        type* type_ptr = reinterpret_cast<type*>(type_ptr_raw); \
        return type_ptr->linkObject(); \
    }\
    static Heart::hObjectDefinition autogen_entity_definition_##name = { \
        type::getRuntimeTypeID(), \
        type::getTypeNameStatic(), \
        sizeof(type),\
        autogen_construct_##name, \
        autogen_destruct_##name, \
        autogen_destroy_##name, \
        autogen_create_serialiser_##name, \
        autogen_serialise_##name, \
        autogen_link_##name, \
    };\
    hBool type::auto_object_registered = Heart::hObjectFactory::objectFactoryRegistar(&autogen_entity_definition_##name, #serialiser_type, ##__VA_ARGS__, nullptr)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
    
namespace hObjectFactory {
    const hObjectDefinition*        getObjectDefinition(hStringID name);
    const hObjectDefinition*        getObjectDefinitionFromSerialiserName(const hChar* name);
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
