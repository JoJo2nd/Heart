/********************************************************************

    filename:   DebugPrimsTest.h  
    
    Copyright (c) 2:2:2013 James Moran
    
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

#ifndef DEBUGPRIMSTEST_H__
#define DEBUGPRIMSTEST_H__

#include "UnitTestFactory.h"

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
    Heart::hParameterConstantBlock* viewportCB_;
    Heart::hParameterConstantBlock* modelMtxCB_;
    Heart::hMatrix                   modelMtx_;

    //Camera Control
    Heart::hVec3            camPos_;
    Heart::hVec3            camDir_;
    Heart::hVec3            camUp_;
};

#endif // DEBUGPRIMSTEST_H__