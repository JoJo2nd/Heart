/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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