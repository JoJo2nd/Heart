/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef HRAPIDXML_H__
#define HRAPIDXML_H__

#include "base/hTypes.h"
#include "base/hMemory.h"
#include "base/hRendererConstants.h"
#include "base/hStringUtil.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"

namespace Heart
{
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    void* HEART_API hXML_alloc_func(size_t size );
    void HEART_API hXML_free_func(void* ptr );


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class hXMLDocument : public rapidxml::xml_document<>
    {
    public:
        hXMLDocument()
            : data_(NULL)
        {
            set_allocator( hXML_alloc_func, hXML_free_func );
        }

        ~hXMLDocument()
        {
            hFree(data_); data_ = nullptr;
        }

        template< hUint32 flags >
        hBool ParseSafe(hChar* data)
        {
            hFree(data_); data_ = nullptr;
            data_ = data;
            try {
                parse< flags >( data );
            } catch( ... ) {
                return hFalse;
            }
            return hTrue;
        }

    private:

        hXMLDocument( const hXMLDocument& rhs );
        hXMLDocument& operator = ( const hXMLDocument& rhs );

        hChar*           data_;
    };

    template< typename t_enum >
    struct hXMLEnumReamp
    {
        hXMLEnumReamp() {}
        hXMLEnumReamp(const std::string& name, t_enum value) 
            : enumStr_(name)
            , enumValue_(value) {}
        std::string  enumStr_;
        t_enum       enumValue_;
    };

    class hXMLGetter
    {
    public:
        hXMLGetter() 
            : node_(NULL)
        {
        }
        explicit hXMLGetter( const rapidxml::xml_node<>* node )
            : node_(node)
        {
            
        }

        void                    SetNode(const rapidxml::xml_node<>* node)
        {
            node_ = node;
        }

        hXMLGetter              FirstChild( const hChar* name ) const
        {
            if ( !node_ )
                return hXMLGetter( NULL );

            return hXMLGetter( node_->first_node( name ) );
        }
        hXMLGetter              NextSibling() const
        {
            if ( !node_ || !node_->parent() )
                return hXMLGetter(NULL);

            return hXMLGetter(node_->next_sibling(node_->name()));
        }
        hXMLGetter              NextSiblingAny() const
        {
            if ( !node_ || !node_->parent() )
                return hXMLGetter(NULL);

            return hXMLGetter(node_->next_sibling(NULL));
        }
        hXMLGetter              NextSibling( const hChar* name ) const
        {
            if ( !node_ || !node_->parent() )
                return hXMLGetter(NULL);

            return hXMLGetter(node_->next_sibling(name));
        }
        rapidxml::xml_attribute<>*  GetAttribute( const hChar* name ) const
        {
            if ( !node_ )
                return NULL;

            return node_->first_attribute( name );
        }
        hInt32 GetAttributeInt(const hChar* name) const
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
        hInt32 GetAttributeInt(const hChar* name, hInt32 defVal) const
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
        hFloat GetAttributeFloat(const hChar* name) const
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
        hFloat GetAttributeFloat(const hChar* name, hFloat defVal) const
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
        const hChar* GetAttributeString(const hChar* name) const
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
        const hChar* GetAttributeString(const hChar* name, const hChar* defVal) const
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
        t_type GetAttributeEnum(const hChar* name, hXMLEnumReamp<t_type>* enums, t_type defaultVal = 0) const
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
                if (hStrICmp(enums->enumStr_.c_str(), att->value()) == 0)
                {
                    return (t_type)enums->enumValue_;
                }
                ++enums;
            }

            return defaultVal;
        }
        hInt32 GetValueInt(hInt32 defaultVal = 0) const
        {
            if (!node_)
                return defaultVal;
            if (!node_->value())
                return defaultVal;
            return hAtoI(node_->value());
        }
        hFloat GetValueFloat(hFloat defaultVal = 0.f) const
        {
            if (!node_)
                return defaultVal;
            if (!node_->value())
                return defaultVal;
            return hAtoF(node_->value());
        }
        const hChar* GetValueString(const hChar* defaultVal = NULL) const
        {
            if (!node_)
                return defaultVal;
            if (!node_->value())
                return defaultVal;
            return node_->value();
        }
        hUint GetValueStringLen() const {
            if (!node_) {
                return 0;
            } else if (!node_->value()) {
                return 0;
            } else {
                return (hUint)node_->value_size();
            }
        }
        template < typename t_type >
        t_type GetValueEnum(hXMLEnumReamp<t_type>* enums, t_type defaultVal = 0) const
        {
            if (!node_)
                return defaultVal;
            if (!node_->value())
                return defaultVal;
            while(enums->enumStr_)
            {
                if (hStrICmp(enums->enumStr_, node_->value()) == 0)
                {
                    return enums->enumValue_;
                }
                ++enums;
            }

            return defaultVal;
        }
        hUint32 GetValueHex(hUint32 hexVal) const
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
        hColour GetValueColour(hColour defVal) const
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

        const rapidxml::xml_node<>*       ToNode() { return node_; }
        hXMLGetter                        parent() const { 
            if (!node_) {
                return hXMLGetter(NULL);
            }
            return hXMLGetter(node_->parent());
        }
        const hChar* getName() const {
            return node_->name();
        }
        hUint getNameLen() const {
            return (hUint)node_->name_size();
        }

    private:

        const rapidxml::xml_node<>*       node_;
    };

    template <>
    hFORCEINLINE bool hXMLGetter::GetValueEnum<bool>(hXMLEnumReamp<bool>* enums, bool defaultVal) const
    {
        if (!node_)
            return defaultVal;
        if (!node_->value())
            return defaultVal;
        while(enums->enumStr_.c_str())
        {
            if (hStrICmp(enums->enumStr_.c_str(), node_->value()) == 0)
            {
                return enums->enumValue_ == 1;
            }
            ++enums;
        }

        return defaultVal;
    }
}

#endif // HRAPIDXML_H__