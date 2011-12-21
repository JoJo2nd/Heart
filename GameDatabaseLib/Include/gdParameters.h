/********************************************************************

	filename: 	gdParameters.h	
	
	Copyright (c) 29:10:2011 James Moran
	
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

#ifndef GDPARAMETERS_H__
#define GDPARAMETERS_H__

namespace GameData
{
    struct gdDLL_EXPORT gdEnumValue
    {
        gdDEFINE_SAVE_VERSION( 0 )

        gdString    name_;
        gdUint      value_;

        friend class boost::serialization::access;

        template< typename _Ty >
        void serialize( _Ty& arc, const unsigned int version )
        {
            switch ( version )
            {
            case 0:
                arc & BOOST_SERIALIZATION_NVP(name_);
                arc & BOOST_SERIALIZATION_NVP(value_);
                break;
            }
        }
    };

    typedef std::vector< gdEnumValue > gdEnumValueArray;

    class gdDLL_EXPORT gdParameterValue
    {
    public:

        enum Type
        {
            Type_Max = 6,
            Type_Int = 0,
            Type_Float,
            Type_Enum,
            Type_String,
            Type_Bool,
            Type_Filename,
        };

        gdParameterValue();
        explicit gdParameterValue( Type type );
        explicit gdParameterValue( const gdEnumValueArray& enumValues );
        ~gdParameterValue();

        gdBool                  GetAsBool() const;
        gdInt                   GetAsInt() const;
        gdFloat                 GetAsFloat() const;
        const gdChar*           GetAsString() const;
        const gdChar*           GetAsFilepath() const;
        const gdEnumValue&      GetAsEnumValue() const; 
        gdUint                  GetEnumValueCount() const;
        const gdEnumValue&      GetEnumValue( gdUint idx ) const;
        void                    Set( gdBool val );
        void                    Set( gdInt val );
        void                    Set( gdFloat val );
        void                    SetEnumByValue( gdUint val );
        void                    SetEnumByName( const gdChar* val );
        void                    Set( const gdChar* val );
        void                    SetFile( const gdChar* val );
        gdBool                  IsBool() const;
        gdBool                  IsInt() const;
        gdBool                  IsFloat() const;
        gdBool                  IsEnum() const;
        gdBool                  IsString() const;
        gdBool                  IsFilepath() const;

        const hChar*            ConvertToString() const;

        gdDEFINE_SAVE_VERSION( 1 )

    private:
        // Hide stupid ms warning C4251
        // (XXX has no DLL interface warning. Things that are private 
        // in the class are not inline to avoid exposing them to outside libs.
        // All work with the private members is hidden in the dll implementation.)
#pragma warning( push )
#pragma warning( disable: 4251 )

        friend class boost::serialization::access;

        template< typename _Ty >
        void serialize( _Ty& arc, const unsigned int version )
        {
            switch( version )
            {
            case 1:
                arc & BOOST_SERIALIZATION_NVP(valueFilepath_);
            case 0:
                arc & BOOST_SERIALIZATION_NVP(type_);
                arc & BOOST_SERIALIZATION_NVP(enumValues_);
                arc & BOOST_SERIALIZATION_NVP(valueStr_);
                arc & BOOST_SERIALIZATION_NVP(valueNum_);
                arc & BOOST_SERIALIZATION_NVP(valueIntEnumBool_);
                break;
            }
        }

        Type                    type_;
        gdEnumValueArray        enumValues_;
        gdString                valueStr_;
        gdFloat                 valueNum_;
        gdInt                   valueIntEnumBool_;
        gdString                valueFilepath_;
        mutable gdString        convertedValue_;
#pragma warning ( pop )
    };

    class gdDLL_EXPORT gdParameterDefinition
    {
    public:

        gdParameterDefinition();
        ~gdParameterDefinition();

        const gdChar*           GetName() const;
        void                    SetName( const gdChar* val );
        const gdParameterValue& GetDefaultValue() const;
        void                    SetDefaultValue( const gdParameterValue& val );
        gdBool                  GetIsValid() const;

    private:
        // Hide stupid ms warning C4251
        // (XXX has no DLL interface warning. Things that are private 
        // in the class are not inline to avoid exposing them to outside libs.
        // All work with the private members is hidden in the dll implementation.)
#pragma warning( push )
#pragma warning( disable: 4251 )

        gdString            name_;
        gdParameterValue    defaultValue_;
        gdUint32            flags_;

        //Stop Hiding warning C4251
#pragma warning ( pop )
    };

    class gdDLL_EXPORT gdParameterPair
    {
    public:
        gdParameterPair();
        ~gdParameterPair();
        
        const gdChar*                   GetName() const;
        void                            SetName( const gdChar* name );
        gdParameterValue*               GetValue();
        const gdParameterValue*         GetValue() const;
        void                            SetParameterDefinition( const gdParameterDefinition& paramDef );
        const gdParameterDefinition&    GetParameterDefinition() const;   

        gdDEFINE_SAVE_VERSION( 0 )

    private:
        // Hide stupid ms warning C4251
        // (XXX has no DLL interface warning. Things that are private 
        // in the class are not inline to avoid exposing them to outside libs.
        // All work with the private members is hidden in the dll implementation.)
#pragma warning( push )
#pragma warning( disable: 4251 )

        friend class boost::serialization::access;

        template< typename _Ty >
        void serialize( _Ty& arc, const unsigned int version )
        {
            switch( version )
            {
            case 0:
                arc & BOOST_SERIALIZATION_NVP(name_);
                arc & BOOST_SERIALIZATION_NVP(value_);
                break;
            }
        }

        gdString                name_;//matches the definition name
        gdParameterValue        value_;
        gdParameterDefinition   definition_;

        //Stop Hiding warning C4251
#pragma warning ( pop )
    };
}

#endif // GDPARAMETERS_H__