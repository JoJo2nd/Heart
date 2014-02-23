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

namespace Heart
{
    typedef ::google::protobuf::MessageLite hObjectMarshall;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    typedef void (*hEntityConstructProc)(void*);
    typedef void (*hEntityDestructProc)(void*);
    typedef hObjectMarshall* (*hEntityConstructMarshallProc)();

    struct hObjectDefinition 
    {
        hStringID                       entityName_;
        hSize_t                         typeSize_;
        hEntityConstructProc            construct_;
        hEntityConstructMarshallProc    constructMarshall_;
    };

#define hObjectType(name) \
    static hStringID getTypeNameStatic() { \
        static hStringID typeName(#name); \
        return typeName; \
    } \
    virtual hStringID getTypeName() { \
        return getTypeNameStatic(); \
    }

#define hRegisterObjectType(type, marshall) \
    extern hBool Heart::hEntityFactory::objectFactoryRegistar(hEntityDefinition*); \
    static void autogen_construct_##name (void* ptr) { return hPLACEMENT_NEW(ptr) type(); } \
    static void autogen_construct_marshall_##name (void* ptr) { return hNEW(marshall); } \
    static hEntityDefinition autogen_entity_definition = { \
        type::getTypeNameStatic(), \
        sizeof(type), \
        autogen_construct_##name, \
        autogen_construct_marshall_##name, \
    };\
    static hBool auto_entity_registered_##name = Heart::hEntityFactoryRegistar(&autogen_entity_definition);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
    
namespace hObjectFactory
{
    const hObjectDefinition*        getObjectDefinition(hStringID name);
    void*                           createObject(hStringID name);
    hObjectMarshall*                createObjectMarshall(hStringID name);
    hBool                           objectFactoryRegistar(hObjectDefinition*);
    template< typename t_ty >
    hFORCEINLINE hObjectMarshall*   createObjectMarshall(t_ty* ptr) {
        return createObjectMarshall(ptr->getTypeName());
    }
}

}
#endif // HOBJECTFACTORY_H__
