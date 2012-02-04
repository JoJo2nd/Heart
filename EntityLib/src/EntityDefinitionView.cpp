/********************************************************************

	filename: 	EntityDefinitionView.cpp	
	
	Copyright (c) 3:2:2012 James Moran
	
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

#include "entitylib/EntityLib.h"
#include "entitylib/EntityDefinitionView.h"

namespace Entity
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const IErrorObject EntityDefinitionView::AppendComponent( const IComponentView* component )
    {
        if ( HasComponent( component ) )
            return ENTITYLIB_ERROR( "Component already exists in entity" );

        uint newprops = component->GetDefaultPropertyCount();
        for ( uint i = 0; i < newprops; ++i )
        {
            overrides_.push_back( new OverrideComponentProperty( static_cast< const ComponentProperty* >( component->GetDefaultProperty(i) ), this ) );
        }

        components_.push_back( static_cast< const ComponentView* >( component ) );

        return ENTITYLIB_OK();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const IErrorObject EntityDefinitionView::RemoveComponent( const char* name )
    {
        for ( ComponentVectorType::iterator i = components_.begin(); i != components_.end(); ++i )
        {
            if ( strcmp( (*i)->GetName(), name ) == 0 )
            {                
                for ( PropertyVectorType::iterator prop = overrides_.begin(); prop != overrides_.end(); )
                {
                    if ( (*prop)->GetOwnerComponent() == (*i) )
                    {
                        prop = overrides_.erase(prop);
                    }
                    else 
                    {
                        ++prop;
                    }
                }

                components_.erase( i );
                return ENTITYLIB_OK();
            }
        }

        return ENTITYLIB_ERROR( "Couldn't find component to remove" );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    bool EntityDefinitionView::HasComponent( const IComponentView* component ) const
    {
        for ( uint i = 0; i < components_.size(); ++i )
        {
            if ( components_[i] == component )
                return true;
        }

        return false;
    }

}