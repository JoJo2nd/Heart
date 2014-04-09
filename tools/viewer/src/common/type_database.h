/********************************************************************
    
    Copyright (c) 5:4:2014 James Moran
    
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

namespace types
{
    typedef google::protobuf::Message*            (*TypeConstructProc)();
    typedef const google::protobuf::Descriptor*   (*TypeDecriptorProc)();
    typedef const google::protobuf::Reflection*   (*TypeReflectProc)();

    google::protobuf::Message*              createMessageFromName(const char* );
    const google::protobuf::Descriptor*     getDescriptorFromName(const char* );
    const google::protobuf::Reflection*     getReflectionFromName(const char* );
    bool                                    registerType(const char*, TypeConstructProc, TypeDecriptorProc, TypeReflectProc);
    template < typename t_ty >
    const t_ty* getDefaultTypeObject() {
        return &t_ty::default_instance();
    }
}

#define DEFINE_AND_REGISTER_TYPE(name_space, type_name) \
    namespace types { \
    namespace type_##type_name { \
        google::protobuf::Message* typeCreate() { \
            return getDefaultTypeObject<name_space::type_name>()->New(); \
        } \
        const google::protobuf::Descriptor* typeDescriptor() { \
            return name_space::type_name::descriptor(); \
        } \
        const google::protobuf::Reflection* typeReflection() { \
            return getDefaultTypeObject<name_space::type_name>()->GetReflection(); \
        } \
        bool typeRegistered = registerType(getDefaultTypeObject<name_space::type_name>()->GetTypeName().c_str(), typeCreate, typeDescriptor, typeReflection); \
    } }
