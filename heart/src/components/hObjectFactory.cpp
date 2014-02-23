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
    typedef std::unordered_map< hStringID, hObjectDefinition, hStringIDHash > hObjectDefinitionTable;

    static hBool                   doneGlobalInit_ = hFalse;
    static hObjectDefinitionTable  objectDefTable_;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const hObjectDefinition* getEntityDefinition(hStringID name) {
    const auto definition = objectDefTable_.find(name);
    if (definition == objectDefTable_.end()) {
        return hNullptr;
    }
    return &definition->second;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* createEntity(hStringID name) {
    auto definition = objectDefTable_.find(name);
    if (definition == objectDefTable_.end()) {
        return hNullptr;
    }
    void* ptr=hMalloc(definition->second.typeSize_);
    definition->second.construct_(ptr);
    return ptr;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hObjectMarshall* createEntityMarshall(hStringID name) {
    auto definition = objectDefTable_.find(name);
    if (definition == objectDefTable_.end()) {
        return hNullptr;
    }
    return definition->second.constructMarshall_();;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool entityFactoryRegistar(hObjectDefinition* definition) {
    hcAssertMsg(doneGlobalInit_, "Registering Type too later, this should be done before main()");
    if (doneGlobalInit_ || definition->entityName_.is_default()) {
        return hFalse;
    }
    objectDefTable_[definition->entityName_] = *definition;
    return hTrue;
}

}
}