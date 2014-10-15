/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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
        viewMatrix_ = Heart::hMatrix::identity();
    }
    ~SimpleFirstPersonFlyCamera()
    {}

    void reset(const Heart::hVec3& up, const Heart::hVec3& forward, const Heart::hVec3& position) {
        up_=up;
        forward_=forward;
        look_=forward;
        position_=position;
        viewMatrix_ = Heart::hMatrix::lookAt(Heart::hPoint3(position_), Heart::hPoint3(position_+look_), up_);
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