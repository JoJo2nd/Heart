/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#ifndef SIBENIK_H__
#define SIBENIK_H__

#include "UnitTestFactory.h"
#include "TestUtils.h"

class Sibenik : public IUnitTest
{
    DECLARE_HEART_UNIT_TEST();
public:
    Sibenik(Heart::hHeartEngine* engine) 
        : IUnitTest( engine )
        , state_(eBeginLoad)
    {
        Heart::hMemTracking::TrackPushMarker("Sibenik_Test");
    }
    ~Sibenik() 
    {
        Heart::hMemTracking::TrackPopMarker();
    }

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
    Heart::hResourceHandle  renderModelHandle_;
    // Heart::hDrawCallContext drawCtx_;
    // Heart::hLightingManager deferredLightManager_;
    // Heart::hShaderResourceView* albedoSRV_;
    // Heart::hShaderResourceView* normalSRV_;
    // Heart::hShaderResourceView* specSRV_;
    // Heart::hShaderResourceView* depthSRV_;

    //Camera Control
    SimpleFirstPersonFlyCamera fpCamera_;
};

#endif // SIBENIK_H__