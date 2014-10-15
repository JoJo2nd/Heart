/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#ifndef EVENTTEST_H__
#define EVENTTEST_H__

#include "UnitTestFactory.h"

class EventTest : public IUnitTest
{
    DECLARE_HEART_UNIT_TEST();
public:
    EventTest (Heart::hHeartEngine* engine);
    ~EventTest() {}

    hUint32				RunUnitTest();

private:

    typedef Heart::hPublisher< void(*)(hUint) >                         IntEventType;
    typedef Heart::hPublisher< void(*)(hFloat) >                        FloatEventType;
    typedef Heart::hPublisher< void(*)(hUint, hFloat, const hChar*) >   IntFloatEventType;
    typedef Heart::hPublisher< void(*)(const hChar*) >                  DeferredEventType;

    void intEvtCounter(hUint i)
    {
        hcPrintf(__FUNCTION__" %u", i);
        if (i < 10) intEvent_(i+1);
    }
    void floatEvtCounter(hFloat f)
    {
        hcPrintf(__FUNCTION__" %f", f);
        if (f < 10.f) floatEvent_(f+1);
    }
    void intFloatEvtCounter(hUint i, hFloat f, const hChar* cookie)
    {
        hcPrintf(__FUNCTION__" %u %f (Cookie %s)", i, f, cookie);
        if (f > 0.f) intFloatEvent_(engine_->GetMainEventPublisher(), i+1, f-1, cookie);
        else deferredEvent_(engine_->GetMainEventPublisher(), cookie);
    }
    void deferredEvent(const hChar* cookie)
    {
        hcPrintf("Exit Event Called (Cookie: %s)", cookie);
        SetExitCode(UNIT_TEST_EXIT_CODE_OK);
    }

    Heart::hHeartEngine*    engine_;
    hBool                   done_;
    IntEventType            intEvent_;
    FloatEventType          floatEvent_;
    IntFloatEventType       intFloatEvent_;
    DeferredEventType       deferredEvent_;
};

#endif // EVENTTEST_H__