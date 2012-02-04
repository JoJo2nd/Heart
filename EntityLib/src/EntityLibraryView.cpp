/********************************************************************

	filename: 	EntityLibraryView.cpp	
	
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

#include "entitylib/EntityLibraryView.h"
#include "rapidxml/rapidxml.hpp"
#include "entitylib/ComponentView.h"
#include "entitylib/ComponentProperty.h"

class XMLGetter
{
public:
    XMLGetter( rapidxml::xml_node<>* node )
        : node_(node) 
    {}
    XMLGetter              FirstChild( const char* name )
    {
        if ( !node_ )
            return XMLGetter( NULL );

        return XMLGetter( node_->first_node( name ) );
    }
    XMLGetter               NextSibling( const char* name )
    {
        if ( !node_ )
            return XMLGetter( NULL );

        return XMLGetter( node_->next_sibling( name ) );
    }
    rapidxml::xml_attribute<>*  GetAttribute( const char* name )
    {
        if ( !node_ )
            return NULL;

        return node_->first_attribute( name );
    }
    rapidxml::xml_node<>*       ToNode() { return node_; }
    const char*                 GetNodeValue() 
    {
        static char empty[] = "";

        if (!node_)
            return empty;
        else
            return node_->value();
    }
    const char*                 GetAttributeValue( const char* name ) 
    {
        static char empty[] = "";

        rapidxml::xml_attribute<>* att = GetAttribute( name );

        if ( !att )
            return empty;
        else
            return att->value();
    }

private:

    rapidxml::xml_node<>*       node_;
};

namespace Entity
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const IErrorObject EntityLibraryView::LoadLibrary( const char* path )
    {
        std::ifstream ofs( path );
        try
        {
            // if *this is passed its an invalid xml tag, hence dbroot is
            // created.
            EntityLibraryView& elib = *this;
            // save data to archive
            boost::archive::xml_iarchive oa(ofs);
            // write class instance to archive
            oa >> BOOST_SERIALIZATION_NVP( elib );
            // archive and stream closed when destructors are called
        }
        catch( boost::archive::xml_archive_exception e )
        {
            return ENTITYLIB_ERROR( e.what() );
        }
        catch( ... )
        {
            return ENTITYLIB_ERROR_GENERIC();
        }

        return ENTITYLIB_OK();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    const IErrorObject EntityLibraryView::ImportComponetsFromXML( const char* path )
    {
        FILE* f = fopen( path, "rt" );
        fseek( f, 0, SEEK_END );
        uint size = ftell( f );
        fseek( f, 0, SEEK_SET );

        char* xmlbuf = new char[size+1];
        fread( xmlbuf, size, 1, f );
        fclose( f );
        xmlbuf[size] = 0;

        rapidxml::xml_document<> compXml;

        compXml.parse< rapidxml::parse_default >( xmlbuf );

        for ( XMLGetter componentDef = XMLGetter( &compXml ).FirstChild( "components" ).FirstChild( "component" ); componentDef.ToNode(); componentDef = componentDef.NextSibling( "component" ) )
        {
            //TODO: make this handle re-importing components...
            ComponentView* compView = new ComponentView();
            
            compView->SetName( componentDef.GetAttributeValue( "name" ) );

            for ( XMLGetter properties = componentDef.FirstChild( "properties" ); properties.ToNode(); properties = properties.NextSibling( "properties" ) )
            {
                ComponentProperty newProp;
                static const char* typeEnumNames[] =
                {
                    "Bool",
                    "Int",
                    "Uint",
                    "Float",
                    "String",
                    "ResourceAsset",
                };
                newProp.SetName( properties.FirstChild( "name" ).GetNodeValue() );
                newProp.SetDoc( properties.FirstChild( "doc" ).GetNodeValue() );
                newProp.SetOwner( compView );
                for ( uint i = 0; i < sizeof(typeEnumNames)/sizeof(typeEnumNames[0]); ++i )
                {
                    if ( _stricmp( properties.FirstChild( "type" ).GetNodeValue(), typeEnumNames[i] ) == 0 )
                    {
                        newProp.SetValueType( (PropertyValueType)i );
                    }
                }
                compView->UpdateProperty( newProp );
            }

            components_.push_back( compView );
        }

        delete xmlbuf;

        return ENTITYLIB_OK();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const IErrorObject EntityLibraryView::SaveLibrary( const char* path )
    {
        std::ofstream ofs( path );
        try
        {
            // if *this is passed its an invalid xml tag, hence dbroot is
            // created.
            const EntityLibraryView& elib = *this;
            // save data to archive
            boost::archive::xml_oarchive oa(ofs);
            // write class instance to archive
            oa << BOOST_SERIALIZATION_NVP( elib );
            // archive and stream closed when destructors are called
        }
        catch ( boost::archive::archive_exception e )
        {
            return ENTITYLIB_ERROR( e.what() );
        }
        catch( ... )
        {
            //TODO:
            return ENTITYLIB_ERROR_GENERIC();
        }

        return ENTITYLIB_OK();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const IComponentView* EntityLibraryView::GetComponentViewByName( const char* name ) const
    {
        for ( uint i = 0; i < components_.size(); ++i )
        {
            if ( strcmp( components_[i]->GetName(), name ) == 0 )
            {
                return components_[i];
            } 
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    IEntityDefinitionView* EntityLibraryView::GetEntityDefinitionByName( const char* name ) const
    {
        for ( uint i = 0; i < entities_.size(); ++i )
        {
            if ( strcmp( entities_[i]->GetName(), name ) == 0 )
            {
                return entities_[i];
            }
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const IErrorObject EntityLibraryView::AddEntity( const char* name, IEntityDefinitionView** output )
    {
        std::string lwrName = name;
        std::transform( lwrName.begin(), lwrName.end(), lwrName.begin(), tolower );

        if ( GetEntityDefinitionByName( lwrName.c_str() ) )
        {
            return ENTITYLIB_ERROR( "Entity name already exists" );
        }

        EntityDefinitionView* entity = new EntityDefinitionView();
        entity->SetName( lwrName.c_str() );

        if ( output )
        {
            *output = entity;
        }

        entities_.push_back( entity );

        return ENTITYLIB_OK();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const IErrorObject EntityLibraryView::RemoveEntity( IEntityDefinitionView* entity )
    {
        typedef std::vector< EntityDefinitionView* > Arr;
        for ( Arr::iterator i = entities_.begin(); i != entities_.end(); ++i )
        {
            if ( (*i) == entity )
            {
                entities_.erase( i );
                return ENTITYLIB_OK();
            }
        }

        return ENTITYLIB_ERROR( "Entity not found in library" );
    }

}