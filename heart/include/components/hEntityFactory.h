/********************************************************************

	filename: 	hEntityFactory.h	
	
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
#ifndef HENTITYFACTORY_H__
#define HENTITYFACTORY_H__


namespace Heart
{
    class hIFileSystem;
    class hHeartEngine;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    class HEART_DLLEXPORT hEntityFactory
    {
    public:

        hEntityFactory()
            : fileSystem_(NULL)
            , activeScript_(NULL)
        {

        }
        ~hEntityFactory()
        {

        }

        void                    Initialise(hIFileSystem* fileSystem, hResourceManager* resourceManager, hHeartEngine* engine);
        void RegisterComponent( 
            const hChar* componentName, 
            hUint32 outComponentID, 
            const hComponentProperty* props, 
            hUint32 propCount, 
            ComponentCreateCallback createFunc, 
            ComponentDestroyCallback destroyFunc );
        hComponentFactory*         GetCompontFactory(const hString& name) const;
        hBool                      ActivateWorldScriptObject(hWorldScriptObject* script);
        hWorldScriptObject*        DeactivateWorldScriptObject();
        hEntity*                   CreateWorldObject(const hChar* worldTypeName, const hChar* objectName, hUint32 id=hErrorCode);
        hEntity*                   CreateEmptyWorldObject(const hChar* objectName, hUint32 id=hErrorCode);
        hEntity*                   CreateWorldObject(hEntityInstanceDefinition* entityDef);
        hEntity*                   FindWorldObjectByID(hUint32 id);
        hEntity*                   FindWorldObjectByName(const hChar* name);
        hResourceClassBase*        OnWorldObjectScriptLoad(const hChar* ext, hUint32 resID, hSerialiserFileStream* dataStream, hResourceManager* resManager);
        hUint32                    OnWorldObjectScriptUnload(const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager);

    private:

        hEntityFactory( const hEntityFactory& rhs );
        hEntityFactory& operator = ( const hEntityFactory& rhs );

        typedef hMap< hString, hComponentFactory > ComponentFactoryMapType;
        typedef hVector< hEntity >                 EntityArray;

        hHeartEngine*                engine_;
        hResourceManager*           resourceManager_;
        hIFileSystem*               fileSystem_;
        ComponentFactoryMapType     factoryMap_;
        hWorldScriptObject*         activeScript_;
        EntityArray                 entityArray_;
    };

#define HEART_REGISTER_COMPONENT_FACTORY( ef, c, createFunc, destroyFunc ) \
    ef->RegisterComponent( c::GetComponentName(), c::GetComponentID(), c::GetPropertyArray(), c::GetPropertyCount(), createFunc, destroyFunc );
}

#endif // HENTITYFACTORY_H__