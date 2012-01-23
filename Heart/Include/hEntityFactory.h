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

namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    typedef huFunctor< hComponent*(*)( hEntity* ) >::type	        ComponentCreateCallback;
    typedef huFunctor< void (*)(hComponent*) >::type	ComponentDestroyCallback;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    class hEntityFactory
    {
    public:

        hEntityFactory();
        ~hEntityFactory();

        void RegisterComponent( const hChar* componentName, hUint32** outComponentID, ComponentCreateCallback createFunc, ComponentDestroyCallback destroyFunc );
        void DumpComponentDefintions();

    private:

        void LoadEntityDefinitions();

        hEntityFactory( const hEntityFactory& rhs );
        hEntityFactory& operator = ( const hEntityFactory& rhs );

        
    };
}

#endif // HENTITYFACTORY_H__