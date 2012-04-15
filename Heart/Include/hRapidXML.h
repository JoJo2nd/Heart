/********************************************************************

	filename: 	hRapidXML.h	
	
	Copyright (c) 22:1:2012 James Moran
	
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

#ifndef HRAPIDXML_H__
#define HRAPIDXML_H__

#include "rapidxml\rapidxml.hpp"
#include "rapidxml\rapidxml_print.hpp"

namespace Heart
{
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    inline void* hXML_alloc_func(size_t size )
    {
        return hMalloc( size );
    }
    inline void hXML_free_func(void* ptr )
    {
        hFree( ptr );
    }


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class hXMLDocument : public rapidxml::xml_document<>
    {
    public:
        hXMLDocument()
            : data_(NULL)
            , heap_(NULL)
        {
            set_allocator( hXML_alloc_func, hXML_free_func );
        }

        ~hXMLDocument()
        {
            if (heap_)
            {
                hHeapFreeSafe((*heap_), data_);
            }
        }

        template< hUint32 flags >
        hBool ParseSafe( hChar* data, hMemoryHeap* heap )
        {
            data_ = data;
            heap_ = heap;
            try
            {
                parse< flags >( data );
            }
            catch( ... )
            {
                return hFalse;
            }
            return hTrue;
        }

    private:

        hXMLDocument( const hXMLDocument& rhs );
        hXMLDocument& operator = ( const hXMLDocument& rhs );

        hChar* data_;
        hMemoryHeap* heap_;
    };

    class hXMLGetter
    {
    public:
        hXMLGetter( rapidxml::xml_node<>* node )
            : node_(node)
        {
            
        }

        hXMLGetter              FirstChild( const hChar* name )
        {
            if ( !node_ )
                return hXMLGetter( NULL );

            return hXMLGetter( node_->first_node( name ) );
        }
        hXMLGetter              NextSibling()
        {
            if ( !node_ || !node_->parent() )
                return hXMLGetter(NULL);

            return hXMLGetter(node_->next_sibling(node_->name()));
        }
        hXMLGetter              NextSiblingAny()
        {
            if ( !node_ || !node_->parent() )
                return hXMLGetter(NULL);

            return hXMLGetter(node_->next_sibling(NULL));
        }
        hXMLGetter              NextSibling( const hChar* name )
        {
            if ( !node_ || !node_->parent() )
                return hXMLGetter(NULL);

            return hXMLGetter(node_->next_sibling(name));
        }
        rapidxml::xml_attribute<>*  GetAttribute( const hChar* name )
        {
            if ( !node_ )
                return NULL;

            return node_->first_attribute( name );
        }
        hUint32 GetAttributeInt(const hChar* name)
        {
            rapidxml::xml_attribute<>* att;

            if ( !node_ )
                return NULL;

            att = node_->first_attribute( name );
            if (!att)
                return 0;
            else
                return hAtoI(att->value());
        }
        hFloat GetAttributeFloat(const hChar* name)
        {
            rapidxml::xml_attribute<>* att;
            if ( !node_ )
                return NULL;

            att = node_->first_attribute( name );
            if (!att)
                return 0.f;
            else
                return hAtoF(att->value());
        }
        const hChar* GetAttributeString(const hChar* name)
        {
            rapidxml::xml_attribute<>* att;
            if ( !node_ )
                return NULL;

            att = node_->first_attribute( name );
            if (!att)
                return NULL;
            else
                return att->value();
        }
        rapidxml::xml_node<>*       ToNode() { return node_; }

    private:

        rapidxml::xml_node<>*       node_;
    };
}

#endif // HRAPIDXML_H__