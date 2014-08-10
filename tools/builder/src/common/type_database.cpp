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

#include "precompiled/precompiled.h"
#include "common/type_database.h"

namespace types
{
namespace
{
    struct Type
    {
        TypeConstructProc   construct_;
        TypeDecriptorProc   decriptor_;
        TypeReflectProc     reflection_;
    };

    struct TypeDatabase 
    {
        typedef std::unordered_map< std::string, Type > TypeTable;

        TypeTable database_;

        static TypeDatabase* get() {
            static TypeDatabase instance_;
            return &instance_;
        }

        bool getType(const char* name, Type* out_type_info) {
            auto found_itr = database_.find(name);
            if (found_itr == database_.end()) {
                return false;
            }
            *out_type_info = found_itr->second;
            return true;
        }

        void addType(const char* name, Type in_type) {
            database_.emplace(name, in_type);
        }
    };
}
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    google::protobuf::Message* createMessageFromName(const char* name) {
        auto* db = TypeDatabase::get();
        Type type_info;
        if (!db->getType(name, &type_info)) {
            return nullptr;
        }
        return type_info.construct_();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const google::protobuf::Descriptor* getDescriptorFromName(const char* name) {
        auto* db = TypeDatabase::get();
        Type type_info;
        if (!db->getType(name, &type_info)) {
            return nullptr;
        }
        return type_info.decriptor_();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const google::protobuf::Reflection* getReflectionFromName(const char* name) {
        auto* db = TypeDatabase::get();
        Type type_info;
        if (!db->getType(name, &type_info)) {
            return nullptr;
        }
        return type_info.reflection_();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    bool registerType(const char* name, TypeConstructProc construct_proc, TypeDecriptorProc descriptor_proc, TypeReflectProc reflector_proc) {
        auto* db = TypeDatabase::get();
        Type type_info = {construct_proc, descriptor_proc, reflector_proc};
        db->addType(name, type_info);
        return true;
    }

}