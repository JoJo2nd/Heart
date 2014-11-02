/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#ifndef COMPLEXMESH2_H__
#define COMPLEXMESH2_H__

#include "UnitTestFactory.h"
#include "TestUtils.h"

class ComplexMesh2 : public IUnitTest
{
    DECLARE_HEART_UNIT_TEST();
public:
    ComplexMesh2(Heart::hHeartEngine* engine) 
        : IUnitTest( engine )
        , state_(eBeginLoad)
        , renderModel_(NULL)
    {
    }
    ~ComplexMesh2() {}

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

#endif // COMPLEXMESH2_H__