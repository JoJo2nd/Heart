/********************************************************************

	filename: 	hReflectionUtils.h
	
	Copyright (c) 2011/09/15 James Moran
	
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

#ifndef _HREFLECTIONUTILS_H__
#define _HREFLECTIONUTILS_H__

namespace Heart
{
namespace hReflection
{

// 	template< typename _metatype >
// 	struct PrintMember< hFloat, _metatype >
// 	{
// 		static void Print( const hFloat& val )
// 		{
//             hcPrintf( "hFloat %s::%s (value:%f offset:%u size: %u)", 
//                       _metatype::_ownerclass(),
// 					  _metatype::_name(), 
// 					  val, 
// 					  _metatype::_offset(), 
// 					  _metatype::size );
// 		}
// 	};
// 
//     template< typename _metatype >
//     struct PrintMember< hUint32, _metatype >
//     {
//         static void Print( const hUint32& val )
//         {
//             hcPrintf( "hUint32 %s::%s (value:%u offset:%u size: %u)", 
//                       _metatype::_ownerclass(),
//                       _metatype::_name(), 
//                       val, 
//                       _metatype::_offset(), 
//                       _metatype::size );
//         }
//     };
// 
// 	template< typename _metatype >
// 	struct PrintMember< hInt32, _metatype >
// 	{
// 		static void Print( const hInt32& val )
// 		{
//             hcPrintf( "hInt32 %s::%s (value:%i offset:%u size: %u)", 
//                       _metatype::_ownerclass(),
//                       _metatype::_name(), 
//                       val, 
//                       _metatype::_offset(), 
//                       _metatype::size );
// 		}
// 	};
// 
// 	template< typename _metatype >
// 	struct PrintMember< hUint16, _metatype >
// 	{
// 		static void Print( const hUint16& val )
// 		{
//             hcPrintf( "hUint16 %s::%s (value:%u offset:%u size: %u)", 
//                       _metatype::_ownerclass(),
//                       _metatype::_name(), 
//                       val, 
//                       _metatype::_offset(), 
//                       _metatype::size );
// 		}
// 	};
// 
// 	template< typename _metatype >
// 	struct PrintMember< hInt16, _metatype >
// 	{
// 		static void Print( const hInt16& val )
// 		{
//             hcPrintf( "hInt16 %s::%s (value:%i offset:%u size: %u owner class:%s)", 
//                       _metatype::_ownerclass(),
//                       _metatype::_name(), 
//                       val, 
//                       _metatype::_offset(), 
//                       _metatype::size );
// 		}
// 	};
// 
// 	template< typename _metatype >
// 	struct PrintMember< hChar, _metatype >
// 	{
// 		static void Print( const hChar& val )
// 		{
//             hcPrintf( "hChar %s::%s (value:%c offset:%u size: %u owner class:%s)", 
//                       _metatype::_ownerclass(),
//                       _metatype::_name(), 
//                       val, 
//                       _metatype::_offset(), 
//                       _metatype::size );
// 		}
// 	};
// 
// 	template< typename _metatype >
// 	struct PrintMember< hByte, _metatype >
// 	{
// 		static void Print( const hByte& val )
// 		{
//             hcPrintf( "hByte %s::%s (value:%u offset:%u size: %u owner class:%s)", 
//                       _metatype::_ownerclass(),
//                       _metatype::_name(), 
//                       val, 
//                       _metatype::_offset(), 
//                       _metatype::size );
// 		}
// 	};

    template< typename _type, typename _metatype >
    struct PrintMember
    {
        static void Print( const _type& val )
        {
            hcPrintf( "%s::%s", _metatype::_ownerclass(), _metatype::_name() );
        }
    };

    template< typename _Ty, typename hTypeList, unsigned int i >
    class PrintMembersItr
    {
        typedef hTypeList _typelist;
        typedef _Ty _baseclasstype;

    public:
        static void Func( const _Ty& val )
        {
            typedef hTL::TypeAt< hTypeList, i >::Result typeinfo;
            typedef typeinfo::_memberof memberof;
            typedef typeinfo::_type type;
            typedef typeinfo::_policy policy;

            hcPrintf( "Type %s Pointer", policy::IsPointer() ? "Is" : "Is Not" );
            hcPrintf( "Element Count %u", policy::ElementCount_ );

            //PrintMember< dtype, typeinfo >::Print( *((dtype*)(((hByte*)&val)+typeinfo::_offset())) );
            PrintMember< type, typeinfo >::Print( 
                typeinfo::_policy::GetValue< typeinfo, memberof, type >( val ) );
            PrintMembersItr< _baseclasstype, _typelist, i-1 >::Func( val );
        }
    };

    template< typename _Ty, typename hTypeList >
    class PrintMembersItr< _Ty, hTypeList, 0 >
    {
        typedef hTypeList _typelist;
        typedef _Ty _baseclasstype;

    public:
        static void Func( const _Ty& val )
        {
            typedef hTL::TypeAt< hTypeList, 0 >::Result typeinfo;
            typedef typeinfo::_memberof memberof;
            typedef typeinfo::_type type;
            typedef typeinfo::_policy policy;

            hcPrintf( "Type %s Pointer", policy::IsPointer() ? "Is" : "Is Not" );
            hcPrintf( "Element Count %u", policy::ElementCount_ );
            PrintMember< type, typeinfo >::Print( 
                typeinfo::_policy::GetValue< typeinfo, memberof, type >( val ) );
        }
    };

    template< typename _Ty >
    void TestPrintAllMetaMembers( const _Ty& val )
    {
        typedef _Ty::_meta_attr_list TList;
        PrintMembersItr< _Ty, TList, hTL::Length< TList >::value-1 >::Func( val );
    }
}
}

#endif // _HREFLECTIONUTILS_H__