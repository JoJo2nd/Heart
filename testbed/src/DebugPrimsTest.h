/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#ifndef DEBUGPRIMSTEST_H__
#define DEBUGPRIMSTEST_H__

#include "UnitTestFactory.h"
#include "TestUtils.h"

class DebugPrimsTest : public IUnitTest
{
    DECLARE_HEART_UNIT_TEST();
public:
    DebugPrimsTest(Heart::hHeartEngine* engine) 
        : IUnitTest( engine )
        , state_(eBegin)
    {
    }
    ~DebugPrimsTest() {}

    hUint32				RunUnitTest();
    void                RenderUnitTest();

private:

    enum State
    {
        eBegin,
        eRender,
        eExit,
    };

    void        CreateRenderResources();
    void        DestroyRenderResources();
    void        UpdateCamera();

    State                            state_;
    hFloat                           timer_;
    Heart::hVertexBuffer*            cubeVB_;
    Heart::hMaterialInstance*        wireCubeMat_;
    Heart::hMaterialInstance*        viewLitCubeMat_;
    Heart::hIndexBuffer*             sphereIB_;
    Heart::hVertexBuffer*            sphereVB_;
    Heart::hMaterialInstance*        wireSphereMat_;
    Heart::hMaterialInstance*        viewLitSphereMat_;
    Heart::hVertexBuffer*            coneVB_;
    Heart::hMaterialInstance*        wireConeMat_;
    Heart::hMaterialInstance*        viewLitConeMat_;
    Heart::hIndexBuffer*             planeIB_;
    Heart::hVertexBuffer*            planeVB_;
    Heart::hMaterialInstance*        wirePlaneMat_;
    Heart::hRenderBuffer* viewportCB_;
    Heart::hRenderBuffer* modelMtxCB_;
    Heart::hMatrix                   modelMtx_;

    //Camera Control
    SimpleFirstPersonFlyCamera fpCamera_;
};

#endif // DEBUGPRIMSTEST_H__