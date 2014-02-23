/********************************************************************

    filename:   TestUtils.h  
    
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

#pragma once

#ifndef TESTUTILS_H__
#define TESTUTILS_H__

class SimpleFirstPersonFlyCamera
{
public:
    SimpleFirstPersonFlyCamera() 
        : moveSpeed_(0.5f)
        , turnSpeed_(2.5f)
        , up_(0.f, 1.f, 0.f)
        , forward_(0.f, 0.f, 1.f)
        , look_(0.f, 0.f, 1.f)
        , position_(0.f, 0.f, 0.f)
    {
        viewMatrix_ = Heart::hMatrixFunc::identity();
    }
    ~SimpleFirstPersonFlyCamera()
    {}

    void reset(const Heart::hVec3& up, const Heart::hVec3& forward, const Heart::hVec3& position) {
        up_=up;
        forward_=forward;
        look_=forward;
        position_=position;
        viewMatrix_ = Heart::hMatrixFunc::LookAt(position_, position_+look_, up_);
    }
    void setMoveSpeed(hFloat speed) { moveSpeed_ = speed; }
    void setTurnSpeed(hFloat speed) { turnSpeed_ = speed; }
    //void setInput(const Heart::hdGamepad* pad) { pad_ = pad; }
    void update(hFloat delta);
    const Heart::hMatrix& getViewmatrix() const { return viewMatrix_; }
    const Heart::hVec3& getCameraPosition() const { return position_; }

private:

    hFloat moveSpeed_;
    hFloat turnSpeed_;
    Heart::hVec3 up_;
    Heart::hVec3 forward_;
    Heart::hVec3 look_;
    Heart::hVec3 position_;
    Heart::hMatrix viewMatrix_;
};


#endif // TESTUTILS_H__