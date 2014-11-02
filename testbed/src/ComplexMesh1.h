/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#ifndef COMPLEXMESH1_H__
#define COMPLEXMESH1_H__

#include "UnitTestFactory.h"
#include "TestUtils.h"

class ComplexMesh1 : public IUnitTest
{
    DECLARE_HEART_UNIT_TEST();
public:
    ComplexMesh1(Heart::hHeartEngine* engine) 
        : IUnitTest( engine )
        , state_(eBeginLoad)
        , renderModel_(NULL)
    {
    }
    ~ComplexMesh1() {}

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

    static const hUint MAX_DCS=64;

    void        CreateRenderResources();
    void        DestroyRenderResources();
    void        UpdateCamera();

    State                   state_;
    hFloat                  timer_;
    Heart::hRendererCamera  zPassCamera_;
    Heart::hRendererCamera  camera_;
    Heart::hRenderModel*    renderModel_;
    hUint                   nDrawCalls_;
    Heart::hDrawCall        drawCall_[MAX_DCS];

    //Camera Control
    SimpleFirstPersonFlyCamera fpCamera_;
};

#endif // COMPLEXMESH1_H__