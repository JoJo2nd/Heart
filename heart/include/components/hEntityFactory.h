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
        {

        }
        ~hEntityFactory()
        {
            factoryMap_.Clear(hTrue);
            objectTemplates_.Clear(hTrue);
            entityLookUp_.Clear(hTrue);
        }

        void                    initialise(hIFileSystem* fileSystem, hResourceManager* resourceManager, hHeartEngine* engine);
        void registerComponent( 
            const hChar* componentName, 
            hUint32 outComponentHash, 
            const hComponentProperty* props, 
            ComponentCreateCallback createFunc, 
            ComponentDestroyCallback destroyFunc );
        hComponentFactory*         getCompontFactory(hUint32 compHash) const;
        hEntity*                   createWorldObject(const hChar* wotName, const hChar* wantedObjectName);
        hEntity*                   createWorldObject(const hChar** componentNames, hUint compCount, const hChar* wantedObjectName);
        void                       registerObjectTemplate(const hWorldObjectTemplate& wot);

    private:

        hEntityFactory( const hEntityFactory& rhs );
        hEntityFactory& operator = ( const hEntityFactory& rhs );

        typedef hMap< hUint32, hComponentFactory >      ComponentFactoryMapType;
        typedef hMap< hUint32, hWorldObjectTemplate >   WorldObjectTemplateMapType;
        typedef hMap< hUint32, hEntity >                EntityLookupMap;

        hHeartEngine*               engine_;
        hResourceManager*           resourceManager_;
        hIFileSystem*               fileSystem_;
        ComponentFactoryMapType     factoryMap_;
        WorldObjectTemplateMapType  objectTemplates_;
        EntityLookupMap             entityLookUp_;
    };

#define HEART_REGISTER_COMPONENT_FACTORY( ef, c, createFunc, destroyFunc ) \
    ef->registerComponent( c::GetComponentName(), c::GetComponentHash(), c::GetPropertyArray(), createFunc, destroyFunc );
}

#endif // HENTITYFACTORY_H__