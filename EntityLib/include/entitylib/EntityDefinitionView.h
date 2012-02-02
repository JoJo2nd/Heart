/********************************************************************

	filename: 	EntityDefinitionView.h	
	
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
#ifndef ENTITYDEFINITIONVIEW_H__
#define ENTITYDEFINITIONVIEW_H__

#include "entitylib/EntityLibCommon.h"
#include "entitylib/ComponentView.h"

namespace Entity
{
    class EntityDefinitionView : public IEntityDefinitionView
    {
    public:
        EntityDefinitionView()
        {

        }
        virtual ~EntityDefinitionView()
        {

        }

        const char*             GetName() const { return name_.c_str(); }
        uint                    GetComponentCount() const { return components_.size(); }
        const IComponentView*   GetComponent( uint compIdx ) const { return components_[compIdx]; }
        uint                    GetPropertyCount() const { return overrides_.size(); }
        IComponentProperty*     GetProperty( uint propIdx ) { return overrides_[propIdx]; }

    public:

        friend class boost::serialization::access;

        template< typename _Ty >
        void serialize( _Ty& arc, const unsigned int version )
        {
            switch ( version )
            {
            case 1:
                arc & BOOST_SERIALIZATION_NVP(name_);
                arc & BOOST_SERIALIZATION_NVP(components_);
                arc & BOOST_SERIALIZATION_NVP(overrides_);
            default:
                break;
            }
        }

        typedef std::vector< ComponentView* >                ComponentVectorType;
        typedef std::vector< OverrideComponentProperty* >    PropertyVectorType;

        std::string             name_;
        ComponentVectorType     components_;
        PropertyVectorType      overrides_;     
    };
}

BOOST_CLASS_VERSION( Entity::EntityDefinitionView, 1 )
BOOST_CLASS_EXPORT_KEY( Entity::EntityDefinitionView )
BOOST_CLASS_TRACKING( Entity::EntityDefinitionView, boost::serialization::track_selectively )

#endif // ENTITYDEFINITIONVIEW_H__
