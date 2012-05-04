/********************************************************************

	filename: 	hEntityDataDefinition.h	
	
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
#ifndef HENTITYDATADEFINITION_H__
#define HENTITYDATADEFINITION_H__


namespace Heart
{
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    typedef huFunctor< hComponent*(*)( hEntity* ) >::type	ComponentCreateCallback;
    typedef huFunctor< void (*)(hComponent*) >::type	    ComponentDestroyCallback;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class hComponentFactory : public hMapElement< hString, hComponentFactory >
    {
    public:

        hUint32                   componentID_;
        const hChar*              componentName_;
        hUint32                   componentPropCount_;
        const hComponentProperty* componentProperties_;
        ComponentCreateCallback   createFunc_;
        ComponentDestroyCallback  destroyFunc_;

        const hComponentProperty*       GetProperty(const hChar* name) const
        {
            for (hUint32 i = 0; i< componentPropCount_; ++i)
            {
                if (hStrCmp(componentProperties_[i].name_,name) == 0)
                {
                    return &componentProperties_[i];
                }
            }

            return NULL;
        }
    };

    class hComponentPropertyValue
    {
    public:
        hComponentPropertyValue()
            : type_(NULL)
        {

        }

        ~hComponentPropertyValue()
        {

        }

        const hComponentProperty*   type_;
        hUint32                     size_;
        union
        {
            hBool                   boolValue_;
            hUint32                 uintValue_;
            hInt32                  intValue_;
            hFloat                  floatValue_;
            hChar*                  stringValue_;
            hResourceClassBase*     resourcePointer_;
        }values_;
    };

    class hComponentPropertyValueOverride : public hComponentPropertyValue
    {
    public:
        hComponentPropertyValueOverride()
            : hComponentPropertyValue()
            , compFactory_(NULL)
        {

        }

        hComponentFactory*  compFactory_;
    };

    class hComponentDataDefinition
    {
    public:
        hComponentDataDefinition()
            : baseFactory_(NULL)
        {
        }
        ~hComponentDataDefinition()
        {}

        void SetComponent(hComponentFactory* base) { baseFactory_ = base; }
        hComponentFactory* GetComponentFactory() const { return baseFactory_; }
        void SetPropertyCount(hUint32 count){ properties_.Resize(count); }
        hUint32 GetPropertyCount() const { return properties_.GetSize(); }
        hComponentPropertyValue* GetComponentPropertyDefinition(hUint32 idx) { return &properties_[idx]; }

    private:

        hComponentDataDefinition( const hComponentDataDefinition& rhs );
        hComponentDataDefinition& operator = ( const hComponentDataDefinition& rhs );

        hVector< hComponentPropertyValue >      properties_;
        hComponentFactory*                      baseFactory_;
    };

    class hWorldObjectDefinition : public hMapElement< hUint32, hWorldObjectDefinition >
    {
    public:
        hWorldObjectDefinition()
        {

        }
        ~hWorldObjectDefinition()
        {

        }

        void                        SetName(const hChar* name) { entityTypeName_ = name; }
        const hChar*                GetName() const { return entityTypeName_.c_str(); }
        void                        SetComponentCount(hUint32 count) { componentDefinitions_.Resize(count); }
        hUint32                     GetComponentCount() const { return componentDefinitions_.GetSize(); }
        hComponentDataDefinition*   GetComponentDefinition(hUint32 idx) { return &componentDefinitions_[idx]; }

    private:

        hWorldObjectDefinition( const hWorldObjectDefinition& rhs );
        hWorldObjectDefinition& operator = ( const hWorldObjectDefinition& rhs );

        hString                                 entityTypeName_;
        hVector< hComponentDataDefinition >     componentDefinitions_;
    };

    class hEntityInstanceDefinition
    {
    public:
        
        hString                                     name_;
        hString                                     worldType_;
        hUint32                                     id_;                     // will be hErrorCode if not given
        hVector< hComponentPropertyValueOverride >  propertyOverrides;   // array of overrides

    };

}

#endif // HENTITYDATADEFINITION_H__