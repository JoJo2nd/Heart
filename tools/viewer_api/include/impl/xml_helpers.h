/********************************************************************

    filename:   xml_helpers.h  
    
    Copyright (c) 2:1:2013 James Moran
    
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

#pragma once

#ifndef XML_HELPERS_H__
#define XML_HELPERS_H__

#include "viewer_api_config.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include "boost/shared_ptr.hpp"
#include <stdlib.h>

VAPI_PRIVATE_HEADER();


class xml_doc : public rapidxml::xml_document<>
{
public:

    void copy_parse(const char* data, size_t datasize)
    {
        data_ = boost::shared_ptr< char >(new char[datasize]);
        memcpy(data_.get(), data, datasize);
        parse< rapidxml::parse_default >( data_.get() );
    }

private:

    boost::shared_ptr< char > data_;
};

template< typename t_ty >
struct xml_string_value_pair
{
    const char* string_;
    t_ty        value_;
};

class xml_getter
{
public:
    explicit xml_getter(const xml_doc& xml)
        : node_(xml.first_node())
    {
    }
    explicit xml_getter(rapidxml::xml_node<>* node)
        : node_(node)
    {
    }

    xml_getter              first_child( const char* name ) const
    {
        if ( !node_ )
            return xml_getter( NULL );

        return xml_getter( node_->first_node( name ) );
    }
    xml_getter              first_sibling() const
    {
        if ( !node_ || !node_->parent() )
            return xml_getter(NULL);

        return xml_getter(node_->next_sibling(node_->name()));
    }
    xml_getter              next_sibling() const
    {
        if ( !node_ || !node_->parent() )
            return xml_getter(NULL);

        return xml_getter(node_->next_sibling(NULL));
    }
    xml_getter              first_sibling(const char* name) const
    {
        if ( !node_ || !node_->parent() )
            return xml_getter(NULL);

        return xml_getter(node_->next_sibling(name));
    }
    rapidxml::xml_attribute<>*  get_attribute( const char* name ) const
    {
        if ( !node_ )
            return NULL;

        return node_->first_attribute( name );
    }
    int get_attribute_int(const char* name) const
    {
        rapidxml::xml_attribute<>* att;

        if ( !node_ )
            return NULL;

        att = node_->first_attribute( name );
        if (!att)
            return 0;
        else
            return atoi(att->value());
    }
    int get_attribute_int(const char* name, int defVal) const
    {
        rapidxml::xml_attribute<>* att;

        if ( !node_ )
            return defVal;

        att = node_->first_attribute( name );
        if (!att)
            return defVal;
        else
            return atoi(att->value());
    }
    float get_attribute_float(const char* name) const
    {
        rapidxml::xml_attribute<>* att;
        if ( !node_ )
            return NULL;

        att = node_->first_attribute( name );
        if (!att)
            return 0.f;
        else
            return (float)atof(att->value());
    }
    float get_attribute_float(const char* name, float defVal) const
    {
        rapidxml::xml_attribute<>* att;
        if ( !node_ )
            return defVal;

        att = node_->first_attribute( name );
        if (!att)
            return defVal;
        else
            return (float)atof(att->value());
    }
    const char* get_attribute_string(const char* name) const
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
    const char* get_attribute_string(const char* name, const char* defVal) const
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
    t_type get_attribute(const char* name, xml_string_value_pair< t_type >* enums, const t_type& defaultVal = 0) const
    {
        if (!node_)
            return defaultVal;
        if (!node_->value())float
            return defaultVal;

        rapidxml::xml_attribute<>* att = node_->first_attribute( name );

        if (!att)
            return defaultVal;

        while(enums->string_)
        {
            if (stricmp(enums->string_, att->value()) == 0)
            {
                return enums->value_;
            }
            ++enums;
        }

        return defaultVal;
    }
    int get_value_int(int defaultVal = 0) const
    {
        if (!node_)
            return defaultVal;
        if (!node_->value())
            return defaultVal;
        return atoi(node_->value());
    }
    float get_value_float(float defaultVal = 0.f) const
    {
        if (!node_)
            return defaultVal;
        if (!node_->value())
            return defaultVal;
        return (float)atof(node_->value());
    }
    const char* get_value_string(const char* defaultVal = NULL) const
    {
        if (!node_)
            return defaultVal;
        if (!node_->value())
            return defaultVal;
        return node_->value();
    }
    template < typename t_type >
    t_type get_value(xml_string_value_pair< t_type >* enums, t_type defaultVal = 0) const
    {
        if (!node_)
            return defaultVal;
        if (!node_->value())
            return defaultVal;
        while(enums->string_)
        {
            if (stricmp(enums->string_, node_->value()) == 0)
            {
                return (t_type)enums->value_;
            }
            ++enums;
        }

        return defaultVal;
    }
    size_t get_hex_value(size_t hexVal) const
    {
        size_t ret;
        if (!node_)
            return hexVal;
        if (!node_->value())
            return hexVal;
        if (sscanf(node_->value(), "0x%x", &ret) == 1)
            return ret;
        else
            return hexVal;
    }
    const char* name() 
    {
        if (!node_) return NULL;
        return node_->name();
    }

    rapidxml::xml_node<>*       to_node() { return node_; }
    void operator ++ () 
    {
        node_ = node_->next_sibling();
    }

private:

    rapidxml::xml_node<>*       node_;
};

#endif // XML_HELPERS_H__