/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "testbed_precompiled.h"
#include "EventTest.h"

DEFINE_HEART_UNIT_TEST(EventTest);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

EventTest::EventTest(Heart::hHeartEngine* engine) 
    : IUnitTest( engine )
    , engine_(engine)
    , done_(hFalse)
{
    intEvent_.connect(hFUNCTOR_BINDMEMBER(IntEventType::Delegate, EventTest, intEvtCounter, this));
    floatEvent_.connect(hFUNCTOR_BINDMEMBER(FloatEventType::Delegate, EventTest, floatEvtCounter, this));
    intFloatEvent_.connect(hFUNCTOR_BINDMEMBER(IntFloatEventType::Delegate, EventTest, intFloatEvtCounter, this));
    deferredEvent_.connect(hFUNCTOR_BINDMEMBER(DeferredEventType::Delegate, EventTest, deferredEvent, this));

    Heart::hPublisherContext* ctx = engine_->GetMainEventPublisher();

    intEvent_(1); //Immediate event
    floatEvent_(ctx, 1.f); //Deferred event
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 EventTest::RunUnitTest()
{
    Heart::hPublisherContext* ctx = engine_->GetMainEventPublisher();

    if (!done_) {
        intFloatEvent_(ctx, 1, 9999.f, "Bob"); //Deferred
        intFloatEvent_(ctx, 1, 999.f, "Allan"); //Deferred
        done_ = hTrue;
    }
    return 0;
}
