/********************************************************************

    filename:   EventTest.h  
    
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