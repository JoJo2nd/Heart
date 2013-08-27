/********************************************************************

    filename: 	hEntityFactory.cpp	
    
    Copyright (c) 23:1:2012 James Moran
    
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

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hEntityFactory::initialise(hIFileSystem* fileSystem, hResourceManager* resourceManager, hHeartEngine* engine)
    {
        fileSystem_ = fileSystem;
        resourceManager_ = resourceManager;
        engine_ = engine;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hEntityFactory::registerComponent( 
        const hChar* componentName, 
        hUint32 componentHash, 
        const hComponentProperty* props, 
        ComponentCreateCallback createFunc, 
        ComponentDestroyCallback destroyFunc )
    {
        hComponentFactory* fact = hNEW(hComponentFactory);
        hUint i=0;

        for (; props[i].name_!=NULL; ++i) {}

        fact->componentID_          = componentHash;
        fact->componentName_        = componentName;
        fact->componentPropCount_   = i;
        fact->componentProperties_  = props;
        fact->createFunc_           = createFunc;
        fact->destroyFunc_          = destroyFunc;

        factoryMap_.Insert(componentHash, fact);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hComponentFactory* hEntityFactory::getCompontFactory(hUint32 compHash) const
    {
        return factoryMap_.Find(compHash);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hEntity* hEntityFactory::createWorldObject(const hChar* worldTypeName, const hChar* wantedObjectName)
    {
        hUint32 wotHash=hCRC32::StringCRC(worldTypeName);
        hWorldObjectTemplate* wot = objectTemplates_.Find(wotHash);

        if (!wot) return NULL;

        hEntity* entry;
        hUint nameLen=hStrLen(wantedObjectName)+3;//+3 space for double digit number on end
        hChar* objectName=(hChar*)hAlloca(nameLen);
        hUint32 entryHash=hCRC32::StringCRC(wantedObjectName);
        for (hUint i=0; entityLookUp_.Find(entryHash); ++i, hStrPrintf(objectName, nameLen, "%s%d", wantedObjectName, i), entryHash=hCRC32::StringCRC(objectName)) {}

        //TODO: Allocate from a pool?
        entry = hNEW(hEntity)();
        entry->SetName(objectName);
        entityLookUp_.Insert(entryHash, entry);

        for (hUint32 i = 0; i < wot->getComponentCount(); ++i) {
            hComponentDataDefinition* compDef = wot->getComponentDefinition(i);
            hComponentFactory* compFact = compDef->GetComponentFactory();
            hComponent* comp = compFact->createFunc_(entry);

            comp->SetFactory(compFact);
            entry->AddComponent(comp);

            for(hUint32 i2 = 0; i2 < compDef->GetOverrideCount(); ++i2) {
                const hComponentProperty* prop=compFact->GetProperty(compDef->GetOverrideHash(i2));
                hcAssertMsg(prop, "Unable to find component property for override with hash 0x%08x", compDef->GetOverrideHash(i2));
                if (!prop) continue;

                void* data = compDef->GetOverrideData(i2);
                hUint size = compDef->GetOverrideSize(i2);
                if (prop->flags_&(eComponentTypeFlag_Bool|
                                  eComponentTypeFlag_Float|
                                  eComponentTypeFlag_Int|
                                  eComponentTypeFlag_Matrix|
                                  eComponentTypeFlag_Vector2|
                                  eComponentTypeFlag_Vector3|
                                  eComponentTypeFlag_Vector4|
                                  eComponentTypeFlag_ResID)) {
                    // Just do a memcpy
                    hcAssert(size == prop->size_);
                    hMemCpy((hUint8*)comp+prop->offset_, data, size);
                }
                else if (prop->flags_&eComponentTypeFlag_String) {
                    if (prop->flags_&eComponentTypeFlag_Array) {
                        // copy
                        hcAssert(size < prop->size_);
                        hMemCpy((hUint8*)comp+prop->offset_, data, size);
                    }
                    else if (prop->flags_&eComponentTypeFlag_Ptr) {
                        // alloc & copy
                        hChar** strptr=(hChar**)((hUint8*)comp+prop->offset_);
                        *strptr=hNEW_ARRAY(hChar, size);
                        hMemCpy(*strptr, data, size);
                    }
                }
            }
        }

        entry->SetCreated(hTrue);

        return entry;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hEntity* hEntityFactory::createWorldObject(const hChar** componentNames, hUint compCount, const hChar* wantedObjectName)
    {
        hEntity* entry;
        hUint nameLen=hStrLen(wantedObjectName)+3;//+3 space for double digit number on end
        hChar* objectName=(hChar*)hAlloca(nameLen);
        hUint32 entryHash=hCRC32::StringCRC(wantedObjectName);
        for (hUint i=0; entityLookUp_.Find(entryHash); ++i, hStrPrintf(objectName, nameLen, "%s%d", wantedObjectName, i), entryHash=hCRC32::StringCRC(objectName)) {}

        //TODO: Allocate from a pool?
        entry = hNEW(hEntity)();
        entry->SetName(objectName);
        entityLookUp_.Insert(entryHash, entry);

        for (hUint32 i = 0; i < compCount; ++i) {
            hUint32 compHash=hCRC32::StringCRC(componentNames[i]);
            hComponentFactory* compFact = factoryMap_.Find(compHash);
            hcAssertMsg(compFact, "Couldn't find factor for component named %s", componentNames[i]);
            if (!compFact) continue;
            hComponent* comp = compFact->createFunc_(entry);

            comp->SetFactory(compFact);
            entry->AddComponent(comp);
        }

        entry->SetCreated(hTrue);

        return entry;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hEntityFactory::registerObjectTemplate(const hWorldObjectTemplate& wot)
    {
        hWorldObjectTemplate* newwot=hNEW(hWorldObjectTemplate)();
        *newwot=wot;
        objectTemplates_.Insert(hCRC32::StringCRC(newwot->getName()), newwot);
    }

}