/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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
            TestListElement* p = new Heart::GetGlobalHeap(), TestListElement;
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
            TestListElement* p = new Heart::GetGlobalHeap(), TestListElement;
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
            TestListElement* p = new Heart::GetGlobalHeap(), TestListElement;
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
                list_.Remove( li );
                delete Heart::GetGlobalHeap(), li;
                break;
            }
        }

        li = list_.GetHead();
        list_.Remove(li);
        delete Heart::GetGlobalHeap(), li;
        li = list_.GetTail();
        list_.Remove(li);
        delete Heart::GetGlobalHeap(), li;

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