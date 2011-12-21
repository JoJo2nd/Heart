/********************************************************************

	filename: 	TestReflectionWriter.h
	
	Copyright (c) 2011/09/27 James Moran
	
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
#ifndef _TESTREFLECTIONWRITER_H__
#define _TESTREFLECTIONWRITER_H__

#include "Heart.h"
#include "hSerialiser.h"
#if 0

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

template< typename _Ty, typename _metainfo, typename _IsBaseClass >
struct SerialiseMember
{
    static void Func( const _Ty& /*val*/, Serialiser* /*obj*/ )
    {
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

template< typename _Ty, typename hTypeList, unsigned int i >
class SerialiseItr
{
    typedef hTypeList _typelist;
    typedef _Ty _baseclasstype;

public:

    static void Func( const _Ty& val, Serialiser* ser_ )
    {
        typedef hTL::TypeAt< hTypeList, i >::Result typeinfo;
        typedef typeinfo::_memberof memberof;
        typedef typeinfo::_type type;
        typedef typeinfo::_policy policy;

        SerialiseMember< type, typeinfo, Heart::IsTypeBaseType< type >::Result >::Func( 
            policy::GetValue( val ), ser_ );

        SerialiseItr< _baseclasstype, _typelist, i-1 >::Func( val, ser_ );
    }
};

template< typename _Ty, typename hTypeList >
class SerialiseItr< _Ty, hTypeList, 0 >
{
    typedef hTypeList _typelist;
    typedef _Ty _baseclasstype;

public:
    static void Func( const _Ty& val, Serialiser* ser_ )
    {
        typedef hTL::TypeAt< hTypeList, 0 >::Result typeinfo;
        typedef typeinfo::_memberof memberof;
        typedef typeinfo::_type type;
        typedef typeinfo::_policy policy;

        SerialiseMember< type, typeinfo, Heart::IsTypeBaseType< type >::Result >::Func( policy::GetValue( val ), ser_ );
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

template< typename _Ty, typename _metainfo >
struct SerialiseMember< _Ty, _metainfo, Heart::hNonAtomicType >
{
    static void Func( const _Ty& val, Serialiser* obj )
    {
        typedef _Ty::_meta_attr_list TList;

        for ( hUint32 i = 0; i < _metainfo::_policy::GetElementCount( val ); ++i )
        {
            obj->Seek( _metainfo::_offset()+(i+_metainfo::size), Serialiser::SeekOffset_LASTMARKER );
            obj->PushSeekMarker();

            SerialiseItr< _Ty, TList, hTL::Length< TList >::value-1 >::Func( *(((_Ty*)&val)+i), obj );

            obj->PopSeekMarker();
        }
    }
};

template< typename _Ty, typename _metainfo >
struct SerialiseMember< _Ty, _metainfo, Heart::hNonAtomicTypePtr >
{
    static void Func( const _Ty& val, Serialiser* obj )
    {
        obj->Seek( 0, Serialiser::SeekOffset_END );
        _Ty offset = (_Ty)obj->Tell();
        hUint32 datasize = sizeof(*val)*_metainfo::_policy::GetElementCount( val );
        if ( datasize && val )
        {
            obj->PushSeekMarker();
            obj->Reserve( datasize );

            for ( hUint32 i = 0; i < _metainfo::_policy::GetElementCount( val ); ++i )
            {
                obj->Seek( _metainfo::_offset()+(i+_metainfo::size), Serialiser::SeekOffset_LASTMARKER );
                obj->PushSeekMarker();

                SerialiseItr< _Ty, TList, hTL::Length< TList >::value-1 >::Func( *(val+i), obj );

                obj->PopSeekMarker();
            }

            obj->PopSeekMarker();
        }
        else
        {
            offset = NULL;
        }

        obj->Seek( _metainfo::_offset(), Serialiser::SeekOffset_LASTMARKER );
        obj->Write( &offset, sizeof(_Ty) );
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

template< typename _Ty, typename _metainfo >
struct SerialiseMember< _Ty, _metainfo, Heart::hAtomicType >
{
    static void Func( const _Ty& val, Serialiser* obj )
    {
        obj->Seek( _metainfo::_offset(), Serialiser::SeekOffset_LASTMARKER );
        obj->Write( &val, _metainfo::size*_metainfo::_policy::GetElementCount( val ) );
    }
};

template< typename _Ty, typename _metainfo >
struct SerialiseMember< _Ty, _metainfo, Heart::hAtomicTypePtr >
{
    static void Func( const _Ty& val, Serialiser* obj )
    {
        obj->Seek( 0, Serialiser::SeekOffset_END );
        _Ty offset = (_Ty)obj->Tell();
        hUint32 datasize = sizeof(*val)*_metainfo::_policy::GetElementCount( val );
        if ( datasize && val )
        {
            obj->Reserve( datasize );
            obj->Write( val, datasize );
        }
        else
        {
            offset = NULL;
        }

        obj->Seek( _metainfo::_offset(), Serialiser::SeekOffset_LASTMARKER );
        obj->Write( &offset, sizeof(_Ty) );
    }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

template< typename _Ty >
void Serialise( const _Ty& val, Serialiser* s )
{
#ifdef HEART_USE_REFLECTION
    typedef _Ty::_meta_attr_list TList;

    s->Reserve( sizeof(_Ty) );
    SerialiseItr< _Ty, TList, hTL::Length< TList >::value-1 >::Func( val, s );
#else
    (void)val;
    (void)s;
#endif
}
#endif
#endif // _TESTREFLECTIONWRITER_H__