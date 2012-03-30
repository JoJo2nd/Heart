/********************************************************************

	filename: 	hEntityFactory.cpp	
	
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

#include "Common.h"
#include "hEntityFactory.h"
#include "hRapidXML.h"
#include "hIFileSystem.h"
#include "hIFile.h"

namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hEntityFactory::Initialise( hIFileSystem* fileSystem )
    {
        fileSystem_ = fileSystem;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hEntityFactory::RegisterComponent( 
        const hChar* componentName, 
        hUint32* outComponentID, 
        const hComponentProperty* props, 
        hUint32 propCount, 
        ComponentCreateCallback createFunc, 
        ComponentDestroyCallback destroyFunc )
    {
        hComponentFactory* fact = hNEW(hGeneralHeap, hComponentFactory);
        //TODO:
        // fact->componentID_ = GetNewComponentID();
        // *outComponentID = fact->componentID_;
        fact->componentName_        = componentName;
        fact->componentPropCount_   = propCount;
        fact->componentProperties_  = props;
        fact->createFunc_           = createFunc;
        fact->destroyFunc_          = destroyFunc;

        factoryMap_.Insert( componentName, fact );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hEntityFactory::DumpComponentDefintions()
    {
        /*
        hXMLDocument comxml;

        rapidxml::xml_node<>* root = comxml.allocate_node( rapidxml::node_element, "components" );
        comxml.append_node( root );

        for ( hComponentFactory* i = factoryMap_.GetHead(); i; i = i->GetNext() )
        {
            rapidxml::xml_node<>* comp = comxml.allocate_node( rapidxml::node_element, "component" );
            comp->append_attribute( comxml.allocate_attribute( "name", i->componentName_ ) );
            for ( hUint32 prop = 0; prop < i->componentPropCount_; ++prop )
            {
                rapidxml::xml_node<>* propsnode = comxml.allocate_node( rapidxml::node_element, "properties" );
                propsnode->append_node( comxml.allocate_node( rapidxml::node_element, "name", i->componentProperties_[prop].name_ ) );
                propsnode->append_node( comxml.allocate_node( rapidxml::node_element, "doc",  i->componentProperties_[prop].doc_ ) );
                propsnode->append_node( comxml.allocate_node( rapidxml::node_element, "type", i->componentProperties_[prop].typeStr_ ) );

                comp->append_node( propsnode );
            }

            root->append_node( comp );
        }

        static const hUint32 buffersize = 2*1024*1024;
        hChar* buf = (hChar*)hMalloc( buffersize );
        hChar* end = rapidxml::print( buf, comxml );
        hUint32 size = (hUint32)(end - buf);

        hcAssert( size < buffersize );

        hIFile* file = fileSystem_->OpenFileRoot( "COMPONENTS.XML", FILEMODE_WRITE );
        file->Write( buf, size );
        fileSystem_->CloseFile( file );

        hFree( buf );
        */
    }

}