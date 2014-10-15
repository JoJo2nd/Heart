/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#ifndef LOADTEXTURETEST_H__
#define LOADTEXTURETEST_H__

#include "UnitTestFactory.h"
#include "TestUtils.h"

class LoadTextureTest : public IUnitTest
{
    DECLARE_HEART_UNIT_TEST();
public:
    LoadTextureTest(Heart::hHeartEngine* engine) 
        : IUnitTest( engine )
        , state_(eBeginLoad)
    {
    }
    ~LoadTextureTest() {}

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
    Heart::hRendererCamera           camera_;
    Heart::hTexture*                 resTex_;
    Heart::hShaderResourceView*      resTesSRV_;
    Heart::hRenderBuffer*  modelMtxCB_;
    Heart::hMaterialInstance*        materialInstance_;
    Heart::hIndexBuffer*             quadIB_;
    Heart::hVertexBuffer*            quadVB_;

    //Camera Control
    SimpleFirstPersonFlyCamera fpCamera_;
};

#endif // LoadTextureTest_H__