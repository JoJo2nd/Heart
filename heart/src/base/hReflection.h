/********************************************************************

	filename: 	hReflection.h	
	
	Copyright (c) 25:8:2011 James Moran
	
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

#ifndef HREFLECTION_H__
#define HREFLECTION_H__

namespace Heart
{
	typedef hNullType hMetaNull;

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // Runtime eval of type being a pointer ///////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    
    struct hPointerType {};
    struct hNonPointerType {};

    template< typename _Ty >
    struct hIsPointer
    {
        typedef hNonPointerType TypeResult;
        enum { value_ = 0 };
    };

    template< typename _Ty >
    struct hIsPointer< _Ty* >
    {
        typedef hPointerType TypeResult;
        enum { value_ = 1 };
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    struct hNonArrayType {};
    struct hArrayType {};

    template< typename _Ty >
    struct hIsArray
    {
        typedef _Ty          RawType;
        typedef hNonArrayType TypeResult;
        enum 
        { 
            ElementCount = 1,
            Result = false 
        };
    };

    template< typename _Ty, hUint32 arraysize >
    struct hIsArray< _Ty [arraysize] >
    {
        typedef _Ty       RawType;
        typedef hArrayType TypeResult;
        enum 
        { 
            ElementCount = arraysize,
            Result = true 
        };
    };

    template< typename _Ty, hUint32 arraysize >
    struct hIsArray< const _Ty [arraysize] >
    {
        typedef _Ty       RawType;
        typedef hArrayType TypeResult;
        enum 
        { 
            ElementCount = arraysize,
            Result = true 
        };
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // Classes used to identify atomic types //////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    typedef hMetaNull hNonAtomicType;
    struct hNonAtomicTypePtr {};
    struct hAtomicType {};
    struct hAtomicTypePtr {};

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // Classes to solve type logic ////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    template< typename _Ty >
    struct IsTypeBaseType
    {
        typedef hNonAtomicType Result;
    };

    template< typename _Ty >
    struct IsTypeBaseType< _Ty* >
    {
        typedef hNonAtomicTypePtr Result;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // Atomic Class Versions //////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct IsTypeBaseType< hUint64 >
    {
        typedef hAtomicType Result;
    };

    template<>
    struct IsTypeBaseType< hUint64* >
    {
        typedef hAtomicTypePtr Result;
    };

    template<>
    struct IsTypeBaseType< hUint32 >
    {
        typedef hAtomicType Result;
    };

    template<>
    struct IsTypeBaseType< hUint32* >
    {
        typedef hAtomicTypePtr Result;
    };

    template<>
    struct IsTypeBaseType< hInt32 >
    {
        typedef hAtomicType Result;
    };

    template<>
    struct IsTypeBaseType< hInt32* >
    {
        typedef hAtomicTypePtr Result;
    };

    template<>
    struct IsTypeBaseType< hUint16 >
    {
        typedef hAtomicType Result;
    };

    template<>
    struct IsTypeBaseType< hUint16* >
    {
        typedef hAtomicTypePtr Result;
    };

    template<>
    struct IsTypeBaseType< hInt16 >
    {
        typedef hAtomicType Result;
    };

    template<>
    struct IsTypeBaseType< hInt16* >
    {
        typedef hAtomicTypePtr Result;
    };

    template<>
    struct IsTypeBaseType< hInt >
    {
        typedef hAtomicType Result;
    };

    template<>
    struct IsTypeBaseType< hInt* >
    {
        typedef hAtomicTypePtr Result;
    };

    template<>
    struct IsTypeBaseType< hByte >
    {
        typedef hAtomicType Result;
    };

    template<>
    struct IsTypeBaseType< hByte* >
    {
        typedef hAtomicTypePtr Result;
    };

    template<>
    struct IsTypeBaseType< hChar >
    {
        typedef hAtomicType Result;
    };

    template<>
    struct IsTypeBaseType< hChar* >
    {
        typedef hAtomicTypePtr Result;
    };

    template<>
    struct IsTypeBaseType< hBool >
    {
        typedef hAtomicType Result;
    };

    template<>
    struct IsTypeBaseType< hBool* >
    {
        typedef hAtomicTypePtr Result;
    };

    template<>
    struct IsTypeBaseType< hFloat >
    {
        typedef hAtomicType Result;
    };

    template<>
    struct IsTypeBaseType< hFloat* >
    {
        typedef hAtomicTypePtr Result;
    };

    template<>
    struct IsTypeBaseType< hDouble >
    {
        typedef hAtomicType Result;
    };

    template<>
    struct IsTypeBaseType< hDouble* >
    {
        typedef hAtomicTypePtr Result;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // Policies ///////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
       
    template< typename _Ty, typename _metaTy >
    class hPolicyNormal 
    {
        typedef _Ty _type;
        typedef _metaTy _metatype;
 
    public:
        template< typename _memberof >
        static hFORCEINLINE const _type& GetValue( const _memberof& v )
        {
            return *(_type*)(((hByte*)&v)+_metatype::_offset());
        }

        static hFORCEINLINE hBool IsPointer()
        {
            return hIsPointer< _Ty >::value_;
        }

        static hFORCEINLINE hUint32 GetElementCount( const _type& v )
        {
            return 1;
        }
    };

    template< typename _Ty >
    struct hPolicyAlwaysNULL 
    {
        typedef _Ty _type;

    public:
        template< typename _memberof >
        static hFORCEINLINE const _type& GetValue( const _memberof& v )
        {
            return NULL;
        }

        static hFORCEINLINE hBool IsPointer()
        {
            return hIsPointer< _Ty >::value_;
        }

        static hFORCEINLINE hUint32 GetElementCount( const _type& v )
        {
            return 1;
        }
    };

    template< typename _Ty, typename _metaTy, hUint32 _size >
    struct hPolicyArray
    {
        typedef _Ty _type;
        typedef _metaTy _metatype;
        enum { ElementCount_ = _size };
    public:
        template< typename _memberof >
        static hFORCEINLINE const _type& GetValue( const _memberof& v )
        {
            return *(_type*)(((hByte*)&v)+_metatype::_offset());
        }

        static hFORCEINLINE hBool IsPointer()
        {
            return hIsPointer< _Ty >::value_;
        }

        static hFORCEINLINE hUint32 GetElementCount( const _type& v )
        {
            return ElementCount_;
        }
    };

    template< typename _Ty, typename _metaTy, typename _SizeMetaTy >
    struct hPolicyDArray
    {
        typedef _Ty _type;
        typedef _metaTy _metatype;
        typedef _SizeMetaTy _smetatype;
        
    public:
        template< typename _memberof >
        static hFORCEINLINE const _type& GetValue( const _memberof& v )
        {
            return *(_type*)(((hByte*)&v)+_metatype::_offset());
        }

        static hFORCEINLINE hBool IsPointer()
        {
            return hIsPointer< _Ty >::value_;
        }
        static hFORCEINLINE hUint32 GetElementCount( const _type& v )
        {
            hInt32 os = (_smetatype::_offset()-_metatype::_offset());
            return  *(hUint32*)(((hByte*)&v)+os);
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

	template< typename name_, typename ctype_, typename atttype_, typename policy_ = hPolicyNormal >
	struct hMetaField
	{
		typedef ctype_ _memberof;
		typedef atttype_ _type;
        typedef policy_ _policy;

		enum { size = sizeof(_type) };
        static hFORCEINLINE const char* _ownerclass() { return _memberof::_name(); }
		static hFORCEINLINE const char* _name() { return name_::_name(); }
		static hFORCEINLINE unsigned int _offset() { return name_::_offset(); }
	};

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef HEART_USE_REFLECTION

    #define hMETA_ID_MEMBERS( name ) \
        static hFORCEINLINE const char* _name() { static const char* r = #name; return r; } \
        static hFORCEINLINE hUint32 _id() { static const hUint32 r = Heart::hCRC32::StringCRC(_name()); return r; } 

    #define hMETA_MEMBER_INFO_VAR( type, var ) \
        struct member_info_##var { \
            typedef type _member_type; \
            hMETA_ID_MEMBERS( klass ) \
            static hFORCEINLINE unsigned int _offset() { static const unsigned int o = (unsigned int)&(((_selftype*)0)->var); return o; } \
        }; \

    #define hMETA_BEGIN( klass ) \
        public:\
	        typedef klass _selftype; \
	        hMETA_ID_MEMBERS( klass ) \
        private:\
        typedef Heart::hMetaNull

    #define hMETA_BEGIN_BASE( klass, baseklass ) \
        public:\
	        typedef klass _selftype; \
	        hMETA_ID_MEMBERS( klass ) \
        private:\
    	    typedef baseklass::_meta_attr_list

//     #define HEART_META_MEMBERS_BEGIN_BASE2( klass, baseklass1, baseklass2 ) \
// 	    typedef klass _selftype; \
// 	    static hFORCEINLINE const char* _name() { static const char* r = #klass; return r; } \
// 	    typedef HEART_TYPELIST_2( baseklass1::_meta_attr_list, baseklass2::_meta_attr_list )
// 
//     #define HEART_META_MEMBERS_BEGIN_BASE3( klass, baseklass1, baseklass2, baseklass3 ) \
// 	    typedef klass _selftype; \
// 	    static hFORCEINLINE const char* _name() { static const char* r = #klass; return r; } \
// 	    typedef HEART_TYPELIST_3( baseklass1::_meta_attr_list, baseklass2::_meta_attr_list, baseklass3::_meta_attr_list )

    #define hMETA_MEMBER( access, type, var ) \
	    prev_member_info_##var; \
    access: \
	    type var; \
    private: \
	    hMETA_MEMBER_INFO_VAR( type, var ) \
	    typedef hTypeList< Heart::hMetaField< member_info_##var, _selftype, type, Heart::hPolicyNormal< type, member_info_##var > >, prev_member_info_##var >

    #define hMETA_MEMBER_ARRAY( access, type, var, size ) \
        prev_member_info_##var; \
    access: \
        type var[size]; \
    private: \
        hMETA_MEMBER_INFO_VAR( type, var ) \
        typedef hTypeList< Heart::hMetaField< member_info_##var, _selftype, type, Heart::hPolicyArray< type, member_info_##var, size > >, prev_member_info_##var >

    #define hMETA_MEMBER_DARRAY( access, type, var, size ) \
        hMETA_MEMBER( access, hUint32, size ) \
        prev_member_info_##var; \
    access: \
        type var; \
    private: \
        struct member_info_##var { \
        typedef type _member_type; \
        static hFORCEINLINE const char* _name() { static const char* r = #var; return r; }  \
        static hFORCEINLINE unsigned int _offset() { static const unsigned int o = (unsigned int)&(((_selftype*)0)->var); return o; } \
        }; \
        typedef hTypeList< Heart::hMetaField< member_info_##var, _selftype, type, Heart::hPolicyDArray< type, member_info_##var, member_info_##size > >, prev_member_info_##var >

    #define hMETA_END() \
	    prev_member_info_##var; \
    public: \
	    typedef prev_member_info_##var _meta_attr_list; \
	    static const unsigned int _meta_attr_count =  hTL::Length< _meta_attr_list >::value;
#else 

    #define hMETA_BEGIN( klass )
    #define hMETA_BEGIN_BASE( klass, baseklass )
    #define HEART_META_MEMBERS_BEGIN_BASE2( klass, baseklass1, baseklass2 )
    #define HEART_META_MEMBERS_BEGIN_BASE3( klass, baseklass1, baseklass2, baseklass3 )
    #define hMETA_MEMBER( access, type, var ) \
        access: \
            type var;
    #define hMETA_MEMBER_ARRAY( access, type, var, size ) \
        access: \
            type var[size];
    #define hMETA_MEMBER_DARRAY( access, type, var, size ) \
        hMETA_MEMBER( access, hUint32, size )\
        access: \
            type var;
    #define hMETA_END()

#endif//HEART_USE_REFLECTION

}

#endif // HREFLECTION_H__