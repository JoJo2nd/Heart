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

#include "hTypes.h"
#include "hComponent.h"
#include "hEntity.h"
#include "hEntityDataDefinition.h"
#include "hString.h"

namespace Heart
{
    class hIFileSystem;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    typedef huFunctor< hComponent*(*)( hEntity* ) >::type	ComponentCreateCallback;
    typedef huFunctor< void (*)(hComponent*) >::type	    ComponentDestroyCallback;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    class hEntityFactory
    {
    public:

        hEntityFactory()
            : fileSystem_(NULL)
        {

        }
        ~hEntityFactory()
        {

        }

        void Initialise( hIFileSystem* fileSystem );
        void RegisterComponent( 
            const hChar* componentName, 
            hUint32* outComponentID, 
            const hComponentProperty* props, 
            hUint32 propCount, 
            ComponentCreateCallback createFunc, 
            ComponentDestroyCallback destroyFunc );
        void DumpComponentDefintions();

    private:

        void LoadEntityDefinitions();

        hEntityFactory( const hEntityFactory& rhs );
        hEntityFactory& operator = ( const hEntityFactory& rhs );

        struct hComponentFactory : public hMapElement< hString, hComponentFactory >
        {
            hUint32                   componentID_;
            const hChar*              componentName_;
            hUint32                   componentPropCount_;
            const hComponentProperty* componentProperties_;
            ComponentCreateCallback   createFunc_;
            ComponentDestroyCallback  destroyFunc_;
        };

        typedef hMap< hString, hComponentFactory > ComponentFactoryMapType;

        hIFileSystem*               fileSystem_;
        ComponentFactoryMapType     factoryMap_;
    };

#define HEART_REGISTER_COMPONENT_FACTORY( ef, c, createFunc, destroyFunc ) \
    ef->RegisterComponent( c::GetComponentName(), c::GetComponentIDAddress(), c::GetPropertyArray(), c::GetPropertyCount(), createFunc, destroyFunc );
}

#endif // HENTITYFACTORY_H__