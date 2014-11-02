/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#ifndef COMPUTETEST_H__
#define COMPUTETEST_H__

#include "UnitTestFactory.h"
#include "TestUtils.h"

class ComputeTest : public IUnitTest
{
    DECLARE_HEART_UNIT_TEST();
public:
    ComputeTest(Heart::hHeartEngine* engine) 
        : IUnitTest( engine )
        , state_(eBeginLoad)
        , renderModel_(NULL)
    {
    }
    ~ComputeTest() {}

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

    State                            state_;
    hFloat                           timer_;
    Heart::hRenderModel*             renderModel_;
    Heart::hDrawCall                 drawCall_;
    Heart::hDrawCallContext          drawCtx_;
    Heart::hComputeObject            computeParams_;
    Heart::hShaderProgram*           computeProg_;
    Heart::hTexture*                 resTex_;
    Heart::hRenderBuffer* noiseParams_;
    Heart::hShaderResourceView*      resTexSRV_;

    //Camera Control
    SimpleFirstPersonFlyCamera fpCamera_;
};

#endif // COMPUTETEST_H__