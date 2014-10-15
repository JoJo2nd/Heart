/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#ifndef INSTANCERENDERTEST_H__
#define INSTANCERENDERTEST_H__

#include "UnitTestFactory.h"
#include "TestUtils.h"

class InstanceRenderTest : public IUnitTest
{
    DECLARE_HEART_UNIT_TEST();
public:
    InstanceRenderTest(Heart::hHeartEngine* engine) 
        : IUnitTest( engine )
        , state_(eBeginLoad)
        , renderModel_(NULL)
    {
    }
    ~InstanceRenderTest() {}

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
    Heart::hVertexBuffer*   instanceStream_;
    Heart::hDrawCall        drawCall_;
    Heart::hDrawCallContext drawCtx_;

    //Camera Control
    SimpleFirstPersonFlyCamera fpCamera_;
};

#endif // INSTANCERENDERTEST_H__