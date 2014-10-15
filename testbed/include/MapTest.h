/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef MAPTEST_H__
#define MAPTEST_H__

#include "UnitTestFactory.h"

class TestElement : public Heart::hMapElement< hUint32, TestElement >
{
public:
	hUint32		value_;
	hUint32		somethingElse_;
};

#define MAPTESTPRINT __noop
//#define MAPTESTPRINT hcPrintf

class MapTest : public IUnitTest
{
    DECLARE_HEART_UNIT_TEST();
public:
	MapTest( Heart::hHeartEngine* engine ) 
		: IUnitTest( engine )
	{

	}
	~MapTest() {}

	virtual hUint32				RunUnitTest()
	{

		{
			hUint32 t[30];
			hUint32 s[1000];
			hUint32 ss = 1000;
			hUint32 tc = sizeof(t)/sizeof(t[0]);

			ss = GenRandomNumberUnique( s, ss, t, tc );

			MapFill( t, tc );
            MAPTESTPRINT("FILL COMPLETE");
			MapEmpty( t, tc );
            MAPTESTPRINT("EMPTY COMPLETE");
		}

		{
			hUint32 t[200];
			hUint32 s[1000];
			hUint32 ss = 1000;
			hUint32 tc = sizeof(t)/sizeof(t[0]);

			ss = GenRandomNumberUnique( s, ss, t, tc );


			MapFill( t, tc );

			map_.Clear( hTrue );
			MAPTESTPRINT( "map cleared" );
		}

        hcPrintf(__FUNCTION__" Test Complete");
        SetExitCode(UNIT_TEST_EXIT_CODE_OK);

		return 0;
	}

	hUint32 GenRandomNumberUnique( hUint32* s, hUint32 ss, hUint32* t, hUint32 tc ) 
	{
		for ( hUint32 i = 0; i < ss; ++i ) s[i] = i;

		for ( hUint32 i = 0; i < tc; ++i )
		{
			hUint32 ri =  rand()%ss;
			t[i] = s[ri];
			s[ri] = s[--ss];
		}	return ss;
	}

	void MapFill( const hUint32* t, hUint32 tc ) 
	{
		//Test Insert
		for ( hUint32 i = 0; i < tc; ++i )
		{
			MAPTESTPRINT( "!==========================================");

            TestElement* v = new Heart::GetGlobalHeap(), TestElement;
			v->value_ = i;
			v->somethingElse_ = ~i;
			MAPTESTPRINT( "Inserting Key,Value (%u,%u)", t[i], i );
			map_.Insert( t[i], v );
			hcAssert( map_.Validate() );

			hUint32 c = 0;
			hUint32 prevKey = 0;

			MAPTESTPRINT( "Going Forward === %u", i+1 );

			c = 0;
			prevKey = 0;
			for ( TestElement* i2 = map_.GetHead(); i2; i2 = i2->GetNext(), ++c )
			{
				hcAssert( prevKey <= i2->GetKey() );
				prevKey = i2->GetKey();
				MAPTESTPRINT( "Walk: (%u,%u)", i2->GetKey(), i2->value_ );
			}

			hcAssert( c == i+1 );
			MAPTESTPRINT( "%u==========================================!",c);

			MAPTESTPRINT( "Going Backward === %u", i+1 );

			c = 0;
			prevKey = ~0U;
			for ( TestElement* i2 = map_.GetTail(); i2; i2 = i2->GetPrev(), ++c )
			{
				hcAssert( prevKey >= i2->GetKey() );
				prevKey = i2->GetKey();
				MAPTESTPRINT( "Walk: (%u,%u)", i2->GetKey(), i2->value_ );
			}

			hcAssert( c == i+1 );
			MAPTESTPRINT( "%u==========================================!",c);
		}
	}

	void MapEmpty( const hUint32* t, hUint32 tc ) 
	{
		for ( hUint32 i = 0; i < tc; ++i )
		{
			//Test Delete
			MAPTESTPRINT( "Removing Key,Value (%u,%u)", t[i], 0 );
            TestElement* v = map_.Remove( t[i] );
			delete Heart::GetGlobalHeap(), v;
			hcAssert( map_.Validate() );

			hUint32 c = 0;
			hUint32 prevKey = 0;

			MAPTESTPRINT( "Going Forward === %u", i );

			c = 0;
			prevKey = 0;
			for ( TestElement* i2 = map_.GetHead(); i2; i2 = i2->GetNext(), ++c )
			{
				hcAssert( prevKey <= i2->GetKey() );
				prevKey = i2->GetKey();
				MAPTESTPRINT( "Walk: (%u,%u)", i2->GetKey(), i2->value_ );
			}

			hcAssert( c == map_.GetSize() );
			MAPTESTPRINT( "%u==========================================!",c);

			MAPTESTPRINT( "Going Backward === %u", i+1 );

			c = 0;
			prevKey = ~0U;
			for ( TestElement* i2 = map_.GetTail(); i2; i2 = i2->GetPrev(), ++c )
			{
				hcAssert( prevKey >= i2->GetKey() );
				prevKey = i2->GetKey();
				MAPTESTPRINT( "Walk: (%u,%u)", i2->GetKey(), i2->value_ );
			}

			hcAssert( c == map_.GetSize() );
			MAPTESTPRINT( "%u==========================================!",c);
		}
	}

private:

	Heart::hMap< hUint32, TestElement >		map_; 
};

#endif // MAPTEST_H__