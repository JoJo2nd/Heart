/********************************************************************

    filename:   TestUtils.cpp  
    
    Copyright (c) 30:1:2013 James Moran
    
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

#include "testbed_precompiled.h"
#include "TestUtils.h"

void updateCameraFirstPerson(hFloat delta, const Heart::hdGamepad& pad, Heart::hVec3* camUp_, Heart::hVec3* camDir_, Heart::hVec3* camPos_)
{
    using namespace Heart;
    using namespace Heart::hVec3Func;

    hVec3 up=*camUp_;
    hVec3 dir=*camDir_;
    hVec3 pos=*camPos_;
    hVec3 camRight = cross(up, dir);
    hVec3 movement, angleXZ, angleYZ;
    hFloat speed = 5.f, angleSpeed = 2.5f;

    camRight = normaliseFast(camRight);

    speed -= pad.GetAxis(HEART_PAD_LEFT_TRIGGER).anologueVal_*5.f;
    speed += pad.GetAxis(HEART_PAD_RIGHT_TRIGGER).anologueVal_*10.f;
    speed *= delta;

    angleSpeed -= pad.GetAxis(HEART_PAD_LEFT_TRIGGER).anologueVal_*.628f;
    angleSpeed += pad.GetAxis(HEART_PAD_RIGHT_TRIGGER).anologueVal_*1.256f;
    angleSpeed *= delta;

    movement =  scale(camRight, pad.GetAxis(HEART_PAD_LEFT_STICKX).anologueVal_*speed);
    movement += scale(dir, pad.GetAxis(HEART_PAD_LEFT_STICKY).anologueVal_*speed);

    movement += (pad.GetButton(HEART_PAD_DPAD_UP).buttonVal_ ? scale(up, speed) : hVec3Func::zeroVector());
    movement += (pad.GetButton(HEART_PAD_DPAD_DOWN).buttonVal_ ? scale(up, -speed) : hVec3Func::zeroVector());

    angleXZ = hMatrixFunc::mult(dir, hMatrixFunc::rotate(angleSpeed*pad.GetAxis(HEART_PAD_RIGHT_STICKX).anologueVal_, up));
    angleYZ = hMatrixFunc::mult(dir, hMatrixFunc::rotate(angleSpeed*pad.GetAxis(HEART_PAD_RIGHT_STICKY).anologueVal_, camRight));

    dir = angleXZ + angleYZ;
    up  = cross(dir, camRight);

    *camPos_ += movement;
    *camDir_ = normaliseFast(dir);
    *camUp_  = normaliseFast(up);
}