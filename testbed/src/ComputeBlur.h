/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#ifndef COMPUTEBLUR_H__
#define COMPUTEBLUR_H__

#include "UnitTestFactory.h"
#include "TestUtils.h"

class ComputeBlur : public IUnitTest
{
    DECLARE_HEART_UNIT_TEST();
public:
    ComputeBlur(Heart::hHeartEngine* engine) 
        : IUnitTest( engine )
        , state_(eBeginLoad)
    {
    }
    ~ComputeBlur() {}

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
    Heart::hRendererCamera           camera_;
    Heart::hRendererCamera           blurCamera_;
    Heart::hTexture*                 resTex_;
    Heart::hTexture*                 rwTexture_;
    Heart::hRenderBuffer*  modelMtxCB_;
    Heart::hMaterialInstance*        materialInstance_;
    Heart::hMaterialInstance*        blurToScreen_;
    Heart::hIndexBuffer*             quadIB_;
    Heart::hVertexBuffer*            quadVB_;
    Heart::hShaderProgram*           blurHCS_;
    Heart::hShaderProgram*           blurVCS_;
    Heart::hComputeObject            blurHozCObj_;
    Heart::hComputeObject            blurVertCObj_;
    Heart::hComputeUAV               blurUAV_;
    Heart::hShaderResourceView*      blurTexSRV_;
    Heart::hShaderResourceView*      computeBlurTexSRV_;
    Heart::hRenderBuffer* blurParamCB_;

    //Camera Control
    SimpleFirstPersonFlyCamera fpCamera_;
};

#endif // ComputeBlur_H__