/********************************************************************

	filename: 	ReflectionTest.cpp
	
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

#include "ReflectionTest.h"
#include "hSerialiser.h"
#include "hSerialiserFileStream.h"

class TestFooNonIntrusive
{
public: 
    hUint64 u64_;

    bool operator == ( const TestFooNonIntrusive& rhs )
    {
        return 
            u64_  == rhs.u64_;
    }
    bool operator != ( const TestFooNonIntrusive& rhs ) { return !(*this == rhs); }
};
namespace Heart
{
    template<>
    void SerialiseMethod< TestFooNonIntrusive >( Heart::hSerialiser* ser, const TestFooNonIntrusive& data )
    {
        SERIALISE_ELEMENT( data.u64_ );
    }

    template<>
    void DeserialiseMethod< TestFooNonIntrusive >( Heart::hSerialiser* ser, TestFooNonIntrusive& data )
    {
        DESERIALISE_ELEMENT( data.u64_ );
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct TestFooSmall
{
    hFloat              f_;
    TestFooNonIntrusive b_;

    bool operator == ( const TestFooSmall& rhs )
    {
        return 
            b_ == rhs.b_ &&
            f_ == rhs.f_;
    }

    bool operator != ( const TestFooSmall& rhs ) { return !(*this == rhs); }
};
namespace Heart
{
    template<>
    void SerialiseMethod< TestFooSmall >( Heart::hSerialiser* ser, const TestFooSmall& data ) 
    {
        SERIALISE_ELEMENT( data.f_ );
        SERIALISE_ELEMENT( data.b_ );
    }

    template<>
    void DeserialiseMethod< TestFooSmall >( Heart::hSerialiser* ser, TestFooSmall& data )
    {
        DESERIALISE_ELEMENT( data.f_ );
        DESERIALISE_ELEMENT( data.b_ );
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class TestFooIntrusive
{
public: 
    hUint64                 u64_;
    hUint32                 u32_;
    hUint16                 u16a_[16];
    TestFooNonIntrusive     st_;
    TestFooSmall            ff_[8];
    hUint32*                pnull_;
    hUint32*                p_;
    hUint32                 npf_;
    hUint32*                pf_;
    TestFooNonIntrusive*    pst_;
    TestFooNonIntrusive*    pstnull_;
    hUint32                 npsta_;
    TestFooNonIntrusive*    psta_;

    bool operator == ( const TestFooIntrusive& rhs )
    {
        for ( hUint32 i = 0; i < 16; ++i )
            if ( u16a_[i] != rhs.u16a_[i] ) return false;
        for ( hUint32 i = 0; i < 8; ++i )
            if ( ff_[i] != rhs.ff_[i] ) return false;
        for ( hUint32 i = 0; i < npf_; ++i )
            if ( pf_[i] != rhs.pf_[i] ) return false;
        for ( hUint32 i = 0; i < npsta_; ++i )
            if ( psta_[i] != rhs.psta_[i] ) return false;
        return 
            st_         == rhs.st_ &&
            u32_        == rhs.u32_ &&
            u64_        == rhs.u64_ &&
            *p_         == *rhs.p_ && 
            pnull_      == rhs.pnull_ &&
            pstnull_    == rhs.pstnull_ &&
            *pst_       == *rhs.pst_ &&
            npsta_      == rhs.npsta_;
    }

    void SerialiseMethod( Heart::hSerialiser* ser ) const
    {
        SERIALISE_ELEMENT( u32_ );
        SERIALISE_ELEMENT( u64_ );
        SERIALISE_ELEMENT( u16a_ );
        SERIALISE_ELEMENT( st_ );
        SERIALISE_ELEMENT( ff_ );
        SERIALISE_ELEMENT( p_ );
        SERIALISE_ELEMENT( pnull_ );
        SERIALISE_ELEMENT( npf_ );
        SERIALISE_ELEMENT_COUNT( pf_, npf_ );
        SERIALISE_ELEMENT( pstnull_ );
        SERIALISE_ELEMENT( pst_ );
        SERIALISE_ELEMENT( npsta_ );
        SERIALISE_ELEMENT_COUNT( psta_, npsta_ );
    }
    void DeserialiseMethod( Heart::hSerialiser* ser )
    {
        DESERIALISE_ELEMENT( u32_ );
        DESERIALISE_ELEMENT( u64_ );
        DESERIALISE_ELEMENT( u16a_ );
        DESERIALISE_ELEMENT( st_ );
        DESERIALISE_ELEMENT( ff_ );
        DESERIALISE_ELEMENT( p_ );
        DESERIALISE_ELEMENT( pnull_ );
        DESERIALISE_ELEMENT( npf_ );
        DESERIALISE_ELEMENT( pf_ );
        DESERIALISE_ELEMENT( pstnull_ );
        DESERIALISE_ELEMENT( pst_ );
        DESERIALISE_ELEMENT( npsta_ );
        DESERIALISE_ELEMENT( psta_ );
    }
};

namespace Heart
{
    template<>
    void SerialiseMethod< TestFooIntrusive >( Heart::hSerialiser* ser, const TestFooIntrusive& data )
    {
        data.SerialiseMethod( ser );
    }

    template<>
    void DeserialiseMethod< TestFooIntrusive >( Heart::hSerialiser* ser, TestFooIntrusive& data )
    {
        data.DeserialiseMethod( ser );
    }
}

typedef TestFooIntrusive TestFoo;

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

hUint32 ReflectionTest::Main()
{
    Heart::hSerialiser          ser;
    TestFoo             saveTest;
    TestFoo             loadTest;

    saveTest.u64_       = 0x8000000010011001;
    saveTest.u32_       = 256; 
    saveTest.pnull_     = NULL;
    saveTest.p_         = new hUint32;
    *saveTest.p_        = 69;
    saveTest.npf_       = 5;
    saveTest.pf_        = new hUint32[saveTest.npf_];
    for ( hUint32 i = 0; i < saveTest.npf_; ++i )
    {
        saveTest.pf_[i] = 42 + i;
    }
    for ( hUint16 i = 0; i < 16; ++i )
        saveTest.u16a_[i] = i+1;
    for ( hUint32 i = 0; i < 8; ++i )
    {
        saveTest.ff_[i].f_ = (hFloat)i;
        saveTest.ff_[i].b_.u64_ = i;
    }
    saveTest.st_.u64_   = 0x8888888810011001;
    saveTest.pstnull_   = NULL;
    saveTest.pst_       = new TestFooNonIntrusive;
    saveTest.pst_->u64_ = 1000;
    saveTest.npsta_     = 10;
    saveTest.psta_      = new TestFooNonIntrusive[saveTest.npsta_];
    for ( hUint32 i = 0; i < saveTest.npsta_ ; ++i )
    {
        saveTest.psta_[i].u64_ = 1001 + i;
    }

    //hcPrintf( "save struct size = %u", sizeof( saveTest )+(sizeof(TestFooNonIntrusive)*saveTest.npsta_)+sizeof(hUint32)*saveTest.npf_+sizeof(TestFooNonIntrusive) );

    Heart::hSerialiserFileStream file;// = engine_->GetFileManager()->OpenFile( "testoutput.bin", Heart::FILEMODE_WRITE );
    file.Open( "testouput.bin", hTrue, engine_->GetFileManager() );
    ser.Serialise( &file, saveTest );
    file.Close();

    file.Open( "testouput.bin", hFalse, engine_->GetFileManager() );
    ser.Deserialise( &file, loadTest );
    file.Close();

    {
//         Heart::hTexture t(NULL);
//         file = engine_->GetFileManager()->OpenFile( "tex.bin", Heart::FILEMODE_WRITE );
//         ser.Serialise( file, t );
//         engine_->GetFileManager()->CloseFile( file );
// 
//         file = engine_->GetFileManager()->OpenFile( "tex.bin", Heart::FILEMODE_READ );
//         ser.Serialise( file, t );
//         engine_->GetFileManager()->CloseFile( file );
    }
  

    hcAssert( saveTest == loadTest );

    delete saveTest.p_;
    delete[] saveTest.pf_;
    delete saveTest.pst_;
    delete[] saveTest.psta_;

    delete loadTest.p_;
    delete[] loadTest.pf_;
    delete loadTest.pst_;
    delete[] loadTest.psta_;

    return Heart::hStateBase::CONTINUE;
}



