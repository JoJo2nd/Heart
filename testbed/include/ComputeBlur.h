/********************************************************************

    filename:   ComputeBlur.h  
    
    Copyright (c) 24:2:2013 James Moran
    
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

#ifndef COMPUTEBLUR_H__
#define COMPUTEBLUR_H__

#include "UnitTestFactory.h"

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
    Heart::hTexture*                 resTex_;
    Heart::hdParameterConstantBlock* modelMtxCB_;
    Heart::hMaterialInstance*        materialInstance_;
    Heart::hIndexBuffer*             quadIB_;
    Heart::hVertexBuffer*            quadVB_;

    //Camera Control
    Heart::hVec3            camPos_;
    Heart::hVec3            camDir_;
    Heart::hVec3            camUp_;
};

#endif // ComputeBlur_H__