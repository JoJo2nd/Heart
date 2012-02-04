/********************************************************************

	filename: 	ComponentProperty.h	
	
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
#ifndef COMPONENTPROPERTY_H__
#define COMPONENTPROPERTY_H__

#include "entitylib/EntityLibCommon.h"
#include <string>

namespace Entity
{
    class EntityDefinitionView;
    class ComponentView;

    class ComponentProperty : public IComponentProperty
    {
    public:
        ComponentProperty() 
            : type_(PropertyValueType_None)
            , owner_(NULL)
        {
            memset( &value_, 0, sizeof(value_) );
        }
        ComponentProperty( const ComponentProperty& rhs )
        {
            memset( &value_, 0, sizeof(value_) );
            name_ = rhs.name_;
            type_ = rhs.type_;
            doc_  = rhs.doc_; 
            owner_ = rhs.owner_;
            if ( rhs.type_ == PropertyValueType_String )
            {
                SetValueString( rhs.GetValueString() );
            }
            else
            {
                value_ = rhs.value_;
            }
        }
        ~ComponentProperty() 
        {
        }
        ComponentProperty& operator = ( const ComponentProperty& rhs )
        {
            name_ = rhs.name_;
            type_ = rhs.type_;
            doc_  = rhs.doc_; 
            owner_ = rhs.owner_;
            if ( rhs.type_ == PropertyValueType_String )
            {
                SetValueString( rhs.GetValueString() );
            }
            else
            {
                value_ = rhs.value_;
            }
            return *this;
        }

        const char*           GetName() const { return name_.c_str(); }
        const char*           GetDoc() const { return doc_.c_str(); }
        const IComponentView* GetOwnerComponent() const { return (IComponentView*)owner_; }
        PropertyValueType     GetValueType() const { return type_; }
        bool                  GetValueBool() const { return value_.asBool_; }
        int                   GetValueInt() const { return value_.asInt_; }
        uint                  GetValueUint() const { return value_.asUint_; }
        const char*           GetValueString() const { return valueStr_.c_str(); }
        float                 GetValueFloat() const { return value_.asFloat_; }
        void                  SetValueBool( bool v ) { value_.asBool_ = v; }
        void                  SetValueInt( int v ) { value_.asInt_ = v; }
        void                  SetValueUint( uint v ) { value_.asUint_ = v; }
        void                  SetValueFloat( float v ) { value_.asFloat_ = v; }
        void                  SetResourceID( uint32 v ) { value_.asResourceID_ = v; }
        void                  SetValueString( const char* v ) 
        { 
            valueStr_ = v; 
        }
        //Our methods (private to this dll)
        void                  SetValueType( PropertyValueType v ) { type_ = v; }
        void                  SetDoc( const char* doc ) { doc_ = doc; }
        void                  SetName( const char* v ) { name_ = v; }
        void                  SetOwner( const ComponentView* component ) { owner_ = component; }

    private:

        friend class boost::serialization::access;

        template< typename _Ty >
        void serialize( _Ty& arc, const unsigned int version )
        {
            BOOST_SERIALIZATION_BASE_OBJECT_NVP( IComponentProperty );

            switch ( version )
            {
            case 1:
                arc & BOOST_SERIALIZATION_NVP(type_);
                arc & BOOST_SERIALIZATION_NVP(name_);
                arc & BOOST_SERIALIZATION_NVP(owner_);
                arc & BOOST_SERIALIZATION_NVP(doc_);
                arc & BOOST_SERIALIZATION_NVP(valueStr_);
                arc & BOOST_SERIALIZATION_NVP(value_.asResourceID_);
            default:
                break;
            }
        }

        PropertyValueType       type_;
        const ComponentView*    owner_;
        std::string             name_;
        std::string             doc_; 
        std::string             valueStr_;
        union 
        {
            bool    asBool_;
            int     asInt_;
            uint    asUint_;
            uint32  asResourceID_;
            float   asFloat_;
        }value_;
    };   

    class OverrideComponentProperty : public IComponentProperty
    {
    public:
       
        OverrideComponentProperty( const ComponentProperty* base, EntityDefinitionView* entity )
            : base_(base)
            , entity_(entity)
            , override_(NULL)
        {

        }
        ~OverrideComponentProperty()
        {
            delete override_;
            override_ = NULL;
        }

        const char*             GetName() const { return base_->GetName(); }
        const char*             GetDoc() const { return base_->GetDoc(); }
        const IComponentView*   GetOwnerComponent() const { return base_->GetOwnerComponent(); }
        PropertyValueType       GetValueType() const { return base_->GetValueType(); }
        bool                    GetValueBool() const { return override_ ? override_->GetValueBool() : base_->GetValueBool(); }
        int                     GetValueInt() const { return override_ ? override_->GetValueInt() : base_->GetValueInt(); }
        uint                    GetValueUint() const { return override_ ? override_->GetValueUint() : base_->GetValueUint(); }
        const char*             GetValueString() const { return override_ ? override_->GetValueString() : base_->GetValueString(); }
        float                   GetValueFloat() const { return override_ ? override_->GetValueFloat() : base_->GetValueFloat(); }
        void                    SetValueBool( bool v ) { if ( !override_ ) GetOverride(); override_->SetValueBool( v ); }
        void                    SetValueInt( int v ) { if ( !override_ ) GetOverride(); override_->SetValueInt( v ); }
        void                    SetValueUint( uint v ) { if ( !override_ ) GetOverride(); override_->SetValueUint( v ); }
        void                    SetValueFloat( float v ) { if ( !override_ ) GetOverride(); override_->SetValueFloat( v ); }
        void                    SetValueString( const char* v ) { if ( !override_ ) GetOverride(); override_->SetValueString( v ); }


    private:

        friend class boost::serialization::access;

        OverrideComponentProperty()
            : base_(NULL)
            , entity_(NULL)
            , override_(NULL)
        {

        }

        void                    GetOverride();
        template< typename _Ty >
        void serialize( _Ty& arc, const unsigned int version )
        {
            BOOST_SERIALIZATION_BASE_OBJECT_NVP( IComponentProperty );

            switch ( version )
            {
            case 1:
                arc & BOOST_SERIALIZATION_NVP(entity_);
                arc & BOOST_SERIALIZATION_NVP(base_);
                arc & BOOST_SERIALIZATION_NVP(override_);
            default:
                break;
            }
        }

        EntityDefinitionView*     entity_;
        const ComponentProperty*  base_;
        ComponentProperty*        override_;
    };
}

BOOST_CLASS_VERSION( Entity::ComponentProperty, 1 )
BOOST_CLASS_EXPORT_KEY( Entity::ComponentProperty )
BOOST_CLASS_TRACKING( Entity::ComponentProperty, boost::serialization::track_selectively )
BOOST_CLASS_VERSION( Entity::OverrideComponentProperty, 1 )
BOOST_CLASS_EXPORT_KEY( Entity::OverrideComponentProperty )
BOOST_CLASS_TRACKING( Entity::OverrideComponentProperty, boost::serialization::track_selectively )

#endif // COMPONENTPROPERTY_H__
