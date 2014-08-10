/********************************************************************

    filename:   hObjectFactory.h  
    
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

#pragma once

#ifndef HOBJECTFACTORY_H__
#define HOBJECTFACTORY_H__

#include "base/hTypes.h"
#include "base/hStringID.h"
#include "base/hProtobuf.h"

namespace Heart
{
    typedef ::google::protobuf::MessageLite hObjectMarshall;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    typedef void* (*hObjectConstructProc)();
    typedef hObjectMarshall* (*hObjectCreateSerialiserProc)();
    typedef hBool (*hObjectSerialiseProc)(void*, hObjectMarshall*);
    typedef hBool (*hObjectDeserialiseProc)(void*, hObjectMarshall*);

    struct hObjectDefinition 
    {
        hStringID                       entityName_;
        hObjectConstructProc            construct_;
        hObjectCreateSerialiserProc     constructMarshall_;
        hObjectSerialiseProc            serialise_;
        hObjectDeserialiseProc          deserialise_;
        hStringID                       serialiserName_;
        std::vector<hStringID>          baseTypes_;
    };

#define hObjectType(name, serialiser_type) \
    static hStringID getTypeNameStatic() { \
        static hStringID typeName(#name); \
        return typeName; \
    } \
    virtual hStringID getTypeName() const { \
        return getTypeNameStatic(); \
    } \
    hBool serialiseObject(serialiser_type*) const; \
    hBool deserialiseObject(serialiser_type*) 

#define hObjectBaseType(x) hObjectBaseTypeInner(#x)

#define hObjectBaseTypeInner(x) (##x)

#define hRegisterObjectType(name, type, serialiser_type, ...) \
    /*struct Heart::hObjectDefinition;*/ \
    /*extern hBool Heart::hObjectFactory::objectFactoryRegistar(Heart::hObjectDefinition*, const char*, ...); */\
    static void* autogen_construct_##name () { return new type; } \
    static hObjectMarshall* autogen_create_serialiser_##name () { return new serialiser_type; } \
    static hBool autogen_serialise_##name(void* type_ptr_raw, hObjectMarshall* msg_raw) { \
        type* type_ptr = reinterpret_cast<type*>(type_ptr_raw); \
        serialiser_type* msg = static_cast<serialiser_type*>(msg_raw); \
        return type_ptr->serialiseObject(msg); \
    } \
    static hBool autogen_deserialise_##name(void* type_ptr_raw, hObjectMarshall* msg_raw) { \
        type* type_ptr = reinterpret_cast<type*>(type_ptr_raw); \
        serialiser_type* msg = static_cast<serialiser_type*>(msg_raw); \
        return type_ptr->deserialiseObject(msg); \
    } \
    static hObjectDefinition autogen_entity_definition_##name = { \
        type::getTypeNameStatic(), \
        autogen_construct_##name, \
        autogen_create_serialiser_##name, \
        autogen_serialise_##name, \
        autogen_deserialise_##name, \
    };\
    static hBool auto_object_registered_##name = Heart::hObjectFactory::objectFactoryRegistar(&autogen_entity_definition_##name, #serialiser_type, ##__VA_ARGS__, nullptr)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
    
namespace hObjectFactory
{
    const hObjectDefinition*        getObjectDefinition(hStringID name);
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
#endif // HOBJECTFACTORY_H__
