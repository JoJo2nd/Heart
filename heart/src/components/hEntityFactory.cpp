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

    void hEntityFactory::Initialise(hIFileSystem* fileSystem, hResourceManager* resourceManager, hHeartEngine* engine)
    {
        fileSystem_ = fileSystem;
        resourceManager_ = resourceManager;
        engine_ = engine;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hEntityFactory::RegisterComponent( 
        const hChar* componentName, 
        hUint32 outComponentID, 
        const hComponentProperty* props, 
        hUint32 propCount, 
        ComponentCreateCallback createFunc, 
        ComponentDestroyCallback destroyFunc )
    {
        hComponentFactory* fact = hNEW(GetGlobalHeap(), hComponentFactory);
        //TODO:
         fact->componentID_ = outComponentID;
        // *outComponentID = fact->componentID_;
        fact->componentName_        = componentName;
        fact->componentPropCount_   = propCount;
        fact->componentProperties_  = props;
        fact->createFunc_           = createFunc;
        fact->destroyFunc_          = destroyFunc;

        factoryMap_.Insert( componentName, fact );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hComponentFactory* hEntityFactory::GetCompontFactory( const hString& name ) const
    {
        return factoryMap_.Find(name);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hEntityFactory::OnWorldObjectScriptLoad( const hChar* ext, hUint32 resID, hSerialiserFileStream* dataStream, hResourceManager* resManager )
    {
        hUint32 buffersize;
        hChar* xmlbuf;
        hBool parseok;
        hXMLDocument xml;
        hWorldScriptObject* worldobjectscript = hNEW(GetGlobalHeap(), hWorldScriptObject);

        buffersize = dataStream->GetTotalSize();
        xmlbuf = (hChar*)hHeapMalloc(GetGlobalHeap()/*!heap*/,buffersize+32);
        dataStream->Read(xmlbuf,buffersize);
        xmlbuf[buffersize] = 0;
        parseok = xml.ParseSafe<rapidxml::parse_default>(xmlbuf,GetGlobalHeap()/*!heap*/);
        hcAssert( parseok );
        
        worldobjectscript->ParseFromXML(xml, this, resManager);

        return worldobjectscript;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hEntityFactory::OnWorldObjectScriptUnload( const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager )
    {
        hWorldScriptObject* wos = static_cast< hWorldScriptObject* >(resource);

        for (hUint32 i = 0, c = wos->GetWorldObjectTypeCount(); i < c; ++i)
        {
            hWorldObjectDefinition* wso = wos->GetWorldObjectType(i);
            for (hUint32 comp = 0, components = wso->GetComponentCount(); comp < components; ++comp)
            {
                hComponentDataDefinition* cdd = wso->GetComponentDefinition(comp);
                for (hUint32 prop = 0, propCount = cdd->GetPropertyCount(); prop < propCount; ++prop)
                {
                    hComponentPropertyValue* cpv = cdd->GetComponentPropertyDefinition(prop);
                    if (cpv->type_->type_ == eComponentPropertyType_ResourceAsset)
                    {
                        cpv->values_.resourcePointer_->DecRef();
                    }
                    else if (cpv->type_->type_ == eComponentPropertyType_String)
                    {
                        hHeapFree(GetGlobalHeap()/*!heap*/, cpv->values_.stringValue_);
                    }
                }
            }
        }

        for (hUint32 i = 0, c = wos->GetObjectInstanceCount(); i < c; ++i)
        {
            hEntityInstanceDefinition* eid = wos->GetEntityInstanceDefinition(i);
            for (hUint32 overrideIdx = 0, overrides = eid->propertyOverrides.GetSize(); overrideIdx < overrides; ++overrideIdx)
            {
                hComponentPropertyValueOverride* cpvo = &eid->propertyOverrides[overrideIdx];
                if (cpvo->type_->type_ == eComponentPropertyType_ResourceAsset)
                {
                    cpvo->values_.resourcePointer_->DecRef();
                }
                else if (cpvo->type_->type_ == eComponentPropertyType_String)
                {
                    hHeapFree(GetGlobalHeap()/*!heap*/, cpvo->values_.stringValue_);
                }
            }
        }
        hDELETE(GetGlobalHeap(), wos);
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hEntityFactory::ActivateWorldScriptObject(hWorldScriptObject* script)
    {
        //Parse the script object to create any world objects that need to be created...
        activeScript_ = script;
        activeScript_->AddRef();

        entityArray_.Resize(activeScript_->GetObjectInstanceCount());

        //Reserve ID's
        for (hUint32 i = 0, c = activeScript_->GetObjectInstanceCount(); i < c; ++i)
        {
            hUint32 id = activeScript_->GetEntityInstanceDefinition(i)->id_;
            if (id != hErrorCode)
            {
                entityArray_[id].SetReserved(hTrue);
            }
        }

        for (hUint32 i = 0, c = activeScript_->GetObjectInstanceCount(); i < c; ++i)
        {
            CreateWorldObject(activeScript_->GetEntityInstanceDefinition(i));
        }

        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hWorldScriptObject* hEntityFactory::DeactivateWorldScriptObject()
    {
        //TODO: deactivate all objects.
        for (hUint32 i = 0, c = entityArray_.GetSize(); i < c; ++i)
        {
            for (hUint32 i2 = 0, c2 = entityArray_[i].GetComponentCount(); i2 < c2; ++i2)
            {
                entityArray_[i].GetComponent(i2)->OnDeactivate();
            }
        }

        //Gave Warning now destroy
        for (hUint32 i = 0, c = entityArray_.GetSize(); i < c; ++i)
        {
            for (hUint32 i2 = 0, c2 = entityArray_[i].GetComponentCount(); i2 < c2; ++i2)
            {
                entityArray_[i].GetComponent(i2)->OnDestroy();
                hComponent* comp = entityArray_[i].GetComponent(i2);
                hComponentFactory* fact = comp->GetFactory();
                entityArray_[i].RemoveComponent(comp);
                fact->destroyFunc_(comp);
            }
        }

        hWorldScriptObject* ret = activeScript_;
        if (activeScript_)
        {
            activeScript_->DecRef();
            activeScript_ = NULL;
        }
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hEntity* hEntityFactory::CreateWorldObject(const hChar* worldTypeName, const hChar* objectName, hUint32 id)
    {
        if (!activeScript_)
            return NULL;
        hWorldObjectDefinition* wot = activeScript_->GetWorldObjectType(worldTypeName);

        if (!wot)
            return NULL;

        hEntity* entry;

        if (id < hErrorCode)
        {
            entry = &entityArray_[id];
        }
        else
        {
            //Need Free list?
            for (hUint32 i = 0, c = entityArray_.GetSize(); i < entityArray_.GetSize(); ++i)
            {
                if (!entityArray_[i].GetCreated() && !entityArray_[i].GetReserved())
                {
                    entry = &entityArray_[i];
                    break;
                }
            }
        }

        entry->SetName(objectName);

        for (hUint32 i = 0; i < wot->GetComponentCount(); ++i)
        {
            hComponentDataDefinition* compDef = wot->GetComponentDefinition(i);
            hComponentFactory* compFact = compDef->GetComponentFactory();
            hComponent* comp = compFact->createFunc_(entry);

            comp->SetFactory(compFact);
            entry->AddComponent(comp);

            for(hUint32 i2 = 0; i2 < compDef->GetPropertyCount(); ++i2)
            {
                hComponentPropertyValue* prop = compDef->GetComponentPropertyDefinition(i2);
                hUint32 offset = prop->type_->offset_;
                hByte* dst = ((hByte*)comp)+offset;
                switch(prop->type_->type_)
                {
                case eComponentPropertyType_Bool: 
                    *((hBool*)dst) = prop->values_.boolValue_;
                    break;
                case eComponentPropertyType_Int:
                    *((hInt32*)dst) = prop->values_.intValue_;
                    break;
                case eComponentPropertyType_UInt:
                    *((hUint32*)dst) = prop->values_.uintValue_;
                    break;
                case eComponentPropertyType_Float:
                    *((hFloat*)dst) = prop->values_.floatValue_;
                    break;
                case eComponentPropertyType_String:
                    *((hChar**)dst) = prop->values_.stringValue_;
                    break;
                case eComponentPropertyType_ResourceAsset:
                    *((hResourceClassBase**)dst) = prop->values_.resourcePointer_;
                    break;
                }
            }
        }

        entry->SetCreated(hTrue);

        return entry;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hEntity* hEntityFactory::CreateWorldObject( hEntityInstanceDefinition* entityDef )
    {
        hEntity* entry = CreateWorldObject(entityDef->worldType_.c_str(), entityDef->name_.c_str(), entityDef->id_);

        for (hUint32 i = 0, c = entityDef->propertyOverrides.GetSize(); i < c; ++i)
        {
            hComponent* comp = entry->FindComponentByID(entityDef->propertyOverrides[i].compFactory_->componentID_);
            if (!comp)
            {
                //Component not created, so make it now;
                comp = entityDef->propertyOverrides[i].compFactory_->createFunc_(entry);
                comp->SetFactory(entityDef->propertyOverrides[i].compFactory_);
                entry->AddComponent(comp);
            }

            hUint32 offset = entityDef->propertyOverrides[i].type_->offset_;
            hByte* dst = ((hByte*)comp)+offset;
            switch(entityDef->propertyOverrides[i].type_->type_)
            {
            case eComponentPropertyType_Bool: 
                *((hBool*)dst) = entityDef->propertyOverrides[i].values_.boolValue_;
                break;
            case eComponentPropertyType_Int:
                *((hInt32*)dst) = entityDef->propertyOverrides[i].values_.intValue_;
                break;
            case eComponentPropertyType_UInt:
                *((hUint32*)dst) = entityDef->propertyOverrides[i].values_.uintValue_;
                break;
            case eComponentPropertyType_Float:
                *((hFloat*)dst) = entityDef->propertyOverrides[i].values_.floatValue_;
                break;
            case eComponentPropertyType_String:
                *((hChar**)dst) = entityDef->propertyOverrides[i].values_.stringValue_;
                break;
            case eComponentPropertyType_ResourceAsset:
                *((hResourceClassBase**)dst) = entityDef->propertyOverrides[i].values_.resourcePointer_;
                break;
            }
        }

        for (hUint32 i = 0; i < entry->GetComponentCount(); ++i)
        {
            entry->GetComponent(i)->OnCreate(engine_);
        }

        return entry;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hEntity* hEntityFactory::FindWorldObjectByID( hUint32 id )
    {
        if (id < entityArray_.GetSize())
            return &entityArray_[id];
        else
            return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hEntity* hEntityFactory::FindWorldObjectByName( const hChar* name )
    {
        for (hUint32 i = 0; i < entityArray_.GetSize(); ++i)
        {
            if (hStrCmp(name, entityArray_[i].GetName()) == 0)
                return &entityArray_[i];
        }

        return NULL;
    }

}