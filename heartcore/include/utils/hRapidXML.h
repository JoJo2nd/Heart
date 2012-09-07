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
    
    HEARTCORE_SLIBEXPORT void* HEART_API hXML_alloc_func(size_t size );
    HEARTCORE_SLIBEXPORT void HEART_API hXML_free_func(void* ptr );


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class HEARTCORE_SLIBEXPORT hXMLDocument : public rapidxml::xml_document<>
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
                hHeapFreeSafe(heap_, data_);
            }
        }

        template< hUint32 flags >
        hBool ParseSafe( hChar* data, hMemoryHeapBase* heap )
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

        hChar*           data_;
        hMemoryHeapBase* heap_;
    };

    struct hXMLEnumReamp
    {
        const hChar* enumStr_;
        hUint32      enumValue_;
    };

    class HEARTCORE_SLIBEXPORT hXMLGetter
    {
    public:
        explicit hXMLGetter( rapidxml::xml_node<>* node )
            : node_(node)
        {
            
        }

        void                    SetNode(rapidxml::xml_node<>* node)
        {
            node_ = node;
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
        hInt32 GetAttributeInt(const hChar* name)
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
        hInt32 GetAttributeInt(const hChar* name, hInt32 defVal)
        {
            rapidxml::xml_attribute<>* att;

            if ( !node_ )
                return defVal;

            att = node_->first_attribute( name );
            if (!att)
                return defVal;
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
        hFloat GetAttributeFloat(const hChar* name, hFloat defVal)
        {
            rapidxml::xml_attribute<>* att;
            if ( !node_ )
                return defVal;

            att = node_->first_attribute( name );
            if (!att)
                return defVal;
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
        const hChar* GetAttributeString(const hChar* name, const hChar* defVal)
        {
            rapidxml::xml_attribute<>* att;
            if ( !node_ )
                return defVal;

            att = node_->first_attribute( name );
            if (!att)
                return defVal;
            else
                return att->value();
        }
        template < typename t_type >
        t_type GetAttributeEnum(const hChar* name, hXMLEnumReamp* enums, t_type defaultVal = 0)
        {
            if (!node_)
                return defaultVal;
            if (!node_->value())
                return defaultVal;

            rapidxml::xml_attribute<>* att = node_->first_attribute( name );

            if (!att)
                return defaultVal;

            while(enums->enumStr_)
            {
                if (hStrICmp(enums->enumStr_, att->value()) == 0)
                {
                    return (t_type)enums->enumValue_;
                }
                ++enums;
            }

            return defaultVal;
        }
        hInt32 GetValueInt(hInt32 defaultVal = 0)
        {
            if (!node_)
                return defaultVal;
            if (!node_->value())
                return defaultVal;
            return hAtoI(node_->value());
        }
        hFloat GetValueFloat(hFloat defaultVal = 0.f)
        {
            if (!node_)
                return defaultVal;
            if (!node_->value())
                return defaultVal;
            return hAtoF(node_->value());
        }
        const hChar* GetValueString(const hChar* defaultVal = NULL)
        {
            if (!node_)
                return defaultVal;
            if (!node_->value())
                return defaultVal;
            return node_->value();
        }
        template < typename t_type >
        t_type GetValueEnum(hXMLEnumReamp* enums, t_type defaultVal = 0)
        {
            if (!node_)
                return defaultVal;
            if (!node_->value())
                return defaultVal;
            while(enums->enumStr_)
            {
                if (hStrICmp(enums->enumStr_, node_->value()) == 0)
                {
                    return (t_type)enums->enumValue_;
                }
                ++enums;
            }

            return defaultVal;
        }
        hUint32 GetValueHex(hUint32 hexVal)
        {
            hUint32 ret;
            if (!node_)
                return hexVal;
            if (!node_->value())
                return hexVal;
            if (sscanf(node_->value(), "0x%x", &ret) == 1)
                return ret;
            else
                return hexVal;
        }
        hColour GetValueColour(hColour defVal)
        {
            hFloat r,b,g,a;
            if (!node_)
                return defVal;
            if (!node_->value())
                return defVal;
            if (sscanf(node_->value(), " %f , %f , %f , %f ", &r, &g, &b, &a) == 4)
                return hColour(r, b, g, a);
            else
                return defVal;
        }

        rapidxml::xml_node<>*       ToNode() { return node_; }

    private:

        rapidxml::xml_node<>*       node_;
    };
}

#endif // HRAPIDXML_H__