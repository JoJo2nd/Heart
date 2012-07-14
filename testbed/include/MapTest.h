/********************************************************************

	filename: 	MapTest.h	
	
	Copyright (c) 7:8:2011 James Moran
	
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

#ifndef MAPTEST_H__
#define MAPTEST_H__

#include "Heart.h"

class TestElement : public Heart::hMapElement< hUint32, TestElement >
{
public:
	hUint32		value_;
	hUint32		somethingElse_;
};

#define MAPTESTPRINT __noop
//#define MAPTESTPRINT hcPrintf

class MapTest : public Heart::hStateBase
{
public:
	MapTest( Heart::HeartEngine* engine ) 
		: hStateBase( "SimpleRoomAddedLight" )
		,engine_( engine )
	{

	}
	~MapTest() {}

	virtual void				PreEnter() {}
	virtual hUint32				Enter() { return Heart::hStateBase::FINISHED; }
	virtual void				PostEnter() {}
	virtual hUint32				Main()
	{

		{
			hUint32 t[30];
			hUint32 s[1000];
			hUint32 ss = 1000;
			hUint32 tc = sizeof(t)/sizeof(t[0]);

			ss = GenRandomNumberUnique( s, ss, t, tc );

			MapFill( t, tc );
			MapEmpty( t, tc );
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

		return Heart::hStateBase::FINISHED;
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

            TestElement* v = hNEW(Heart::GetGlobalHeap(), TestElement);
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
			delete map_.Remove( t[i] );
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
	virtual void				MainRender() {}
	virtual void				PreLeave() {}
	virtual hUint32				Leave() { return Heart::hStateBase::FINISHED; }

private:

	Heart::HeartEngine*						engine_;
	Heart::hMap< hUint32, TestElement >		map_; 
};

#endif // MAPTEST_H__