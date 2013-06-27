/********************************************************************

    filename: 	ModelRenderTest.h	
    
    Copyright (c) 10:10:2012 James Moran
    
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

#ifndef MODELRENDERTEST_H__
#define MODELRENDERTEST_H__

#include "UnitTestFactory.h"
#include "TestUtils.h"

class ModelRenderTest : public IUnitTest
{
    DECLARE_HEART_UNIT_TEST();
public:
    ModelRenderTest(Heart::hHeartEngine* engine) 
        : IUnitTest( engine )
        , state_(eBeginLoad)
        , renderModel_(NULL)
    {
    }
    ~ModelRenderTest() {}

    hUint32				RunUnitTest();
    void                RenderUnitTest();

private:

    enum State
    {
        eBeginLoad,
        eLoading,
        eRender,
        eBeginUnload,
        eExit,
    };

    void        CreateRenderResources();
    void        DestroyRenderResources();
    void        UpdateCamera();

    State                   state_;
    hFloat                  timer_;
    Heart::hRenderModel*    renderModel_;
    Heart::hDrawCall        drawCall_;
    Heart::hDrawCallContext drawCtx_;

    //Camera Control
    SimpleFirstPersonFlyCamera fpCamera_;
};

#endif // MODELRENDERTEST_H__