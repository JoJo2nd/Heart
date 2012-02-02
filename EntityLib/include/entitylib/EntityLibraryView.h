/********************************************************************

	filename: 	EntityLibraryView.h	
	
	Copyright (c) 28:1:2012 James Moran
	
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
#ifndef ENTITYLIBRARYVIEW_H__
#define ENTITYLIBRARYVIEW_H__

#include "entitylib/EntityLibCommon.h"
#include "ComponentView.h"
#include "EntityDefinitionView.h"

namespace Entity
{

    class ComponentView;
    class EntityDefinitionView;

    class EntityLibraryView : public ILibraryView
    {
    public:
        EntityLibraryView()
        {

        }
        ~EntityLibraryView()
        {

        }

        const IErrorObject      LoadLibrary( const char* path );
        const IErrorObject      ImportComponetsFromXML( const char* path );
        const IErrorObject      SaveLibrary( const char* path );
        uint                    GetComponentCount() const { return components_.size(); }
        const IComponentView*   GetComponentView( uint componentIdx ) const { return static_cast< const IComponentView* >( components_[componentIdx] ); }
        const IComponentView*   GetComponentViewByName( const char* name ) const;
        uint                    GetEntityDefinitionCount() const { return entities_.size(); }
        IEntityDefinitionView*  GetEntityDefinition( uint definitionIdx ) const { return static_cast< IEntityDefinitionView* >( entities_[definitionIdx] ); }
        IEntityDefinitionView*  GetEntityDefinitionByName(  const char* name ) const;
        const IErrorObject      AddEntity( const char* name );
    
    private:

        friend class boost::serialization::access;

        template< typename _Ty >
        void serialize( _Ty& arc, const unsigned int version )
        {
            BOOST_SERIALIZATION_BASE_OBJECT_NVP( ILibraryView );

            switch ( version )
            {
            case 1:
                arc & BOOST_SERIALIZATION_NVP(components_);
                arc & BOOST_SERIALIZATION_NVP(entities_);
            default:
                break;
            }
        }

        std::vector< ComponentView* >           components_;
        std::vector< EntityDefinitionView* >    entities_;
    };
}

BOOST_CLASS_VERSION( Entity::EntityLibraryView, 1 )
BOOST_CLASS_EXPORT_KEY( Entity::EntityLibraryView );
BOOST_CLASS_TRACKING( Entity::EntityLibraryView, boost::serialization::track_selectively )

#endif // ENTITYLIBRARYVIEW_H__