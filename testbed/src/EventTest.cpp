/********************************************************************

    filename:   EventTest.cpp  
    
    Copyright (c) 27:12:2012 James Moran
    
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
