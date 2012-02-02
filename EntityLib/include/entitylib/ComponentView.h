/********************************************************************

	filename: 	ComponentView.h	
	
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
#ifndef COMPONENTVIEW_H__
#define COMPONENTVIEW_H__

#include "entitylib/EntityLibCommon.h"
#include "entitylib/ComponentProperty.h"

namespace Entity
{
    class ComponentView : public IComponentView
    {
    public:
        ComponentView() 
        {}
        virtual ~ComponentView() 
        {}

        //IComponentView methods
        const char*                 GetName() const { return name_.c_str(); }
        uint                        GetDefaultPropertyCount() const { return properties_.size(); }
        const IComponentProperty*   GetDefaultProperty( uint propIdx ) const { return &properties_[propIdx]; }
        //Our methods (private to this dll)
        void                        SetName( const char* name ) { name_ = name; }
        ComponentProperty*          FindPropertyByName( const char* name )
        {
            for ( uint i = 0, iend = properties_.size(); i < iend; ++i )
            {
                if ( strcmp( name, properties_[i].GetName() ) == 0 )
                {
                    return &properties_[i];
                }
            }
        }
        void                        AppendProperty( const ComponentProperty& prop ) { properties_.push_back( prop ); }
        void                        RemovePropertyByName( const char* name )
        {
            for ( PropertyVectorType::iterator i = properties_.begin(), iend = properties_.end(); i < iend; ++i )
            {
                if ( strcmp( name, i->GetName() ) == 0 )
                {
                    properties_.erase( i );
                }
            }
        }

    private:

        friend class boost::serialization::access;

        template< typename _Ty >
        void serialize( _Ty& arc, const unsigned int version )
        {
            BOOST_SERIALIZATION_BASE_OBJECT_NVP( IComponentView );

            switch ( version )
            {
            case 1:
                arc & BOOST_SERIALIZATION_NVP(name_);
                arc & BOOST_SERIALIZATION_NVP(properties_);
            default:
                break;
            }
        }

        typedef std::vector< ComponentProperty > PropertyVectorType;

        std::string         name_;
        PropertyVectorType  properties_;
    };
}

BOOST_CLASS_VERSION( Entity::ComponentView, 1 )
BOOST_CLASS_EXPORT_KEY( Entity::ComponentView )
BOOST_CLASS_TRACKING( Entity::ComponentView, boost::serialization::track_selectively )

#endif // COMPONENTVIEW_H__