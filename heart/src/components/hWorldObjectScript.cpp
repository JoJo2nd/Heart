/********************************************************************

	filename: 	hWorldObjectScript.cpp	
	
	Copyright (c) 31:3:2012 James Moran
	
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

#if 0
    void hWorldScriptObject::ParseFromXML(const hXMLDocument& xml, const hEntityFactory* entityFactory, hResourceManager* resManager)
    {
        //Get all world object types and pack into list/array
        //Get all resources to load them, holding pointers 
        //to them as we need to release them later
        // The object vector owns this memory
        objectDefMap_.SetAutoDelete(false);

        for(hXMLGetter node(hXMLGetter(xml.first_node("objectscript")).FirstChild("worldobjecttypes")); node.ToNode(); node = node.NextSibling())
        {
            for (hXMLGetter wobnode(node.FirstChild("objecttype").ToNode()); wobnode.ToNode(); wobnode = wobnode.NextSibling())
            {
                objectDefs_.Resize(objectDefs_.GetSize()+1);
                hWorldObjectDefinition& wob = objectDefs_[objectDefs_.GetSize()-1];
                hUint32 componentcount = wobnode.FirstChild("components").GetAttributeInt("count");
                hXMLGetter componentnodes = wobnode.FirstChild("components").FirstChild("component");
                hUint32 componentIdx = 0;

                wob.SetName(wobnode.GetAttribute("name")->value());
                wob.SetComponentCount(componentcount);
                for (; componentnodes.ToNode(); componentnodes = componentnodes.NextSibling(), ++componentIdx)
                {
                    hComponentDataDefinition& compDef = *wob.GetComponentDefinition(componentIdx);
                    hComponentFactory* compFact = entityFactory->GetCompontFactory(componentnodes.GetAttributeString("type"));
                    hUint32 propertyIdx = 0;
                    hXMLGetter defaultnode(componentnodes.FirstChild("default"));

                    hcAssert(compFact);

                    compDef.SetComponent(compFact);
                    compDef.SetPropertyCount(componentnodes.GetAttributeInt("defaults"));
                    for(; defaultnode.ToNode(); defaultnode = defaultnode.NextSibling(), ++propertyIdx)
                    {
                        hComponentPropertyValue& propvalue = *compDef.GetComponentPropertyDefinition(propertyIdx);

                        propvalue.type_ = compFact->GetProperty(defaultnode.GetAttributeString("name"));
                        if (propvalue.type_)
                        {
                            //check types match!
                            hcAssert(hStrCmp(propvalue.type_->typeStr_, defaultnode.GetAttributeString("type")) == 0);
                            //
                            switch(propvalue.type_->type_)
                            {
                            case eComponentPropertyType_Bool: 
                                propvalue.values_.boolValue_ = hStrCmp(defaultnode.ToNode()->value(), "true") == 0; 
                                break;
                            case eComponentPropertyType_Int:
                                propvalue.values_.intValue_ = hAtoI(defaultnode.ToNode()->value()); 
                                break;
                            case eComponentPropertyType_UInt:
                                propvalue.values_.uintValue_ = hAtoI(defaultnode.ToNode()->value());
                                break;
                            case eComponentPropertyType_Float:
                                propvalue.values_.floatValue_ = hAtoF(defaultnode.ToNode()->value());
                                break;
                            case eComponentPropertyType_String:
                                propvalue.size_ = defaultnode.ToNode()->value_size();
                                propvalue.values_.stringValue_ = (hChar*)hHeapMalloc(GetGlobalHeap()/*!heap*/,propvalue.size_+1);
                                hStrCopy(propvalue.values_.stringValue_,propvalue.size_+1,defaultnode.ToNode()->value());
                                break;
                            case eComponentPropertyType_ResourceAsset:
                                propvalue.values_.resourcePointer_ = 0;//resManager->LoadResourceFromPath(defaultnode.ToNode()->value());
                                break;
                            }
                        }
                    }
                }
            }
        }

        //Fix up the map for quick(er) look up
        for (hUint32 i =0, c = objectDefs_.GetSize(); i < c; ++i)
        {
            objectDefMap_.Insert( hCRC32::StringCRC(objectDefs_[i].GetName()), &objectDefs_[i]);
        }

        for(hXMLGetter node(hXMLGetter(xml.first_node("objectscript")).FirstChild("worldobjects")); node.ToNode(); node = node.NextSibling())
        {
            for (hXMLGetter object(node.FirstChild("object")); object.ToNode(); object = object.NextSibling())
            {
                worldObjectInstances_.Resize(worldObjectInstances_.GetSize()+1);
                hEntityInstanceDefinition* entityDef = &worldObjectInstances_[worldObjectInstances_.GetSize()-1];
                hUint32 overrideIdx = 0;
                hUint32 overrides = object.GetAttributeInt("overrides");
                entityDef->name_ = object.GetAttributeString("name");
                entityDef->worldType_ = object.GetAttributeString("type");
                entityDef->id_ = object.GetAttribute("id") ? object.GetAttributeInt("id") : hErrorCode;
                entityDef->propertyOverrides.Resize(overrides);
                for (hXMLGetter overrider(object.FirstChild("override")); overrider.ToNode(); overrider = overrider.NextSibling(), ++overrideIdx)
                {
                    hComponentPropertyValueOverride* val = &entityDef->propertyOverrides[overrideIdx];
                    hComponentFactory* factory = entityFactory->GetCompontFactory(overrider.GetAttributeString("component"));

                    hcAssert(factory);

                    val->compFactory_ = factory;
                    val->type_ = factory->GetProperty(overrider.GetAttributeString("parameter"));
                    hcAssert(val->type_);

                    switch(val->type_->type_)
                    {
                    case eComponentPropertyType_Bool: 
                        val->values_.boolValue_ = hStrCmp(overrider.ToNode()->value(), "true") == 0; 
                        break;
                    case eComponentPropertyType_Int:
                        val->values_.intValue_ = hAtoI(overrider.ToNode()->value()); 
                        break;
                    case eComponentPropertyType_UInt:
                        val->values_.uintValue_ = hAtoI(overrider.ToNode()->value());
                        break;
                    case eComponentPropertyType_Float:
                        val->values_.floatValue_ = hAtoF(overrider.ToNode()->value());
                        break;
                    case eComponentPropertyType_String:
                        val->size_ = overrider.ToNode()->value_size();
                        val->values_.stringValue_ = (hChar*)hHeapMalloc(GetGlobalHeap()/*!heap*/,val->size_+1);
                        hStrCopy(val->values_.stringValue_,val->size_+1,overrider.ToNode()->value());
                        break;
                    case eComponentPropertyType_ResourceAsset:
                        val->values_.resourcePointer_ = 0;//resManager->LoadResourceFromPath(overrider.ToNode()->value());
                        break;
                    }
                }
            }
        }
    }

#endif
}