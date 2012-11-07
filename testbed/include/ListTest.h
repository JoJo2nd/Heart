/********************************************************************

	filename: 	ListTest.h	
	
	Copyright (c) 13:8:2011 James Moran
	
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
#ifndef LISTTEST_H__
#define LISTTEST_H__

class TestListElement : public Heart::hLinkedListElement< TestListElement >
{
public:
	hUint32		value_;
};

#define LINKTESTPRINT __noop
//#define LINKTESTPRINT hcPrintf

class ListTest : public IUnitTest
{
    DECLARE_HEART_UNIT_TEST();
public:
	ListTest( Heart::hHeartEngine* engine ) 
		: IUnitTest( engine )
	{

	}
	~ListTest() {}

	virtual hUint32 RunUnitTest()
	{
		{
			hUint32 t[200];
			hUint32 s[1000];
			hUint32 ss = 1000;
			hUint32 tc = sizeof(t)/sizeof(t[0]);

			ss = GenRandomNumberUnique( s, ss, t, tc );

			LinkedListFill( t, tc );

			list_.Clear( hTrue );
		}

		{
			hUint32 t[200];
			hUint32 s[1000];
			hUint32 ss = 1000;
			hUint32 tc = sizeof(t)/sizeof(t[0]);

			ss = GenRandomNumberUnique( s, ss, t, tc );

			LinkedListFill2( t, tc );

			list_.Clear( hTrue );
		}

		{
			hUint32 t[200];
			hUint32 s[1000];
			hUint32 ss = 1000;
			hUint32 tc = sizeof(t)/sizeof(t[0]);

			ss = GenRandomNumberUnique( s, ss, t, tc );

			LinkedListFillAndRemoveSome( t, tc );

			list_.Clear( hTrue );
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

	void						LinkedListFill( hUint32* t, hUint32 tc )
	{
		for ( hUint32 i = 0; i < tc; ++i )
		{
			TestListElement* p = hNEW(Heart::GetGlobalHeap(), TestListElement);
			p->value_ = t[i];
			list_.PushBack( p );
		}

		LINKTESTPRINT( "!==========================================" );
		LINKTESTPRINT( "Going Forward" );
		for ( TestListElement* i = list_.GetHead(); i; i = i->GetNext() )
		{
			LINKTESTPRINT( "Ele : %u", i->value_ );
		}

		LINKTESTPRINT( "!==========================================" );
		LINKTESTPRINT( "Going Backward" );
		for ( TestListElement* i = list_.GetTail(); i; i = i->GetPrev() )
		{
			LINKTESTPRINT( "Ele : %u", i->value_ );
		}
	}

	void						LinkedListFill2( hUint32* t, hUint32 tc )
	{
		for ( hUint32 i = 0; i < tc; ++i )
		{
			TestListElement* p = hNEW(Heart::GetGlobalHeap(), TestListElement);
			p->value_ = t[i];
			if ( i % 2 )
				list_.PushFront( p );
			else
				list_.PushBack( p );
		}

		LINKTESTPRINT( "!==========================================" );
		LINKTESTPRINT( "Going Forward" );
		for ( TestListElement* i = list_.GetHead(); i; i = i->GetNext() )
		{
			LINKTESTPRINT( "Ele : %u", i->value_ );
		}

		LINKTESTPRINT( "!==========================================" );
		LINKTESTPRINT( "Going Backward" );
		for ( TestListElement* i = list_.GetTail(); i; i = i->GetPrev() )
		{
			LINKTESTPRINT( "Ele : %u", i->value_ );
		}
	}

	void						LinkedListFillAndRemoveSome( hUint32* t, hUint32 tc )
	{
		for ( hUint32 i = 0; i < tc; ++i )
		{
			TestListElement* p = hNEW(Heart::GetGlobalHeap(), TestListElement);
			p->value_ = t[i];
			list_.PushBack( p );
		}

		LINKTESTPRINT( "!==========================================" );
		LINKTESTPRINT( "Going Forward" );
		for ( TestListElement* i = list_.GetHead(); i; i = i->GetNext() )
		{
			LINKTESTPRINT( "Ele : %u", i->value_ );
		}

		LINKTESTPRINT( "!==========================================" );
		LINKTESTPRINT( "Going Backward" );
		for ( TestListElement* i = list_.GetTail(); i; i = i->GetPrev() )
		{
			LINKTESTPRINT( "Ele : %u", i->value_ );
		}

		LINKTESTPRINT( "!==========================================" );
		LINKTESTPRINT( "Doing Delete" );
		TestListElement* li = list_.GetHead();
		for ( hUint32 i = 0; li; ++i, li = li->GetNext() )
		{
			if ( i == (tc / 2) )
			{
				LINKTESTPRINT( "Deleting Ele : %u", li->value_ );
                hDELETE(Heart::GetGlobalHeap(), list_.Remove( li ));
				break;
			}
		}

		hDELETE(Heart::GetGlobalHeap(), list_.Remove( list_.GetHead() ));
		hDELETE(Heart::GetGlobalHeap(), list_.Remove( list_.GetTail() ));

		LINKTESTPRINT( "!==========================================" );
		LINKTESTPRINT( "Going Forward" );
		for ( TestListElement* i = list_.GetHead(); i; i = i->GetNext() )
		{
			LINKTESTPRINT( "Ele : %u", i->value_ );
		}

		LINKTESTPRINT( "!==========================================" );
		LINKTESTPRINT( "Going Backward" );
		for ( TestListElement* i = list_.GetTail(); i; i = i->GetPrev() )
		{
			LINKTESTPRINT( "Ele : %u", i->value_ );
		}
	}

private:

	Heart::hLinkedList< TestListElement >	list_; 
};

#endif // LISTTEST_H__