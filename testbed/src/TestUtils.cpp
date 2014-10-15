/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "testbed_precompiled.h"
#include "TestUtils.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void SimpleFirstPersonFlyCamera::update(hFloat delta) {
    using namespace Heart;
#if 1
    hcPrintf("Stub");
#else
    if (!pad_) {
        return;
    }
    hFloat yaw, pitch;
    hVec3 camRight = cross(up_, forward_);
    hVec3 movement;

    yaw = pad_->GetAxis(HEART_PAD_RIGHT_STICKX).anologueVal_*turnSpeed_*delta;
    pitch = pad_->GetAxis(HEART_PAD_RIGHT_STICKY).anologueVal_*turnSpeed_*delta;

    look_ = look_*hMatrixFunc::rotate(yaw, up_);
    look_ = look_*hMatrixFunc::rotate(pitch, camRight);
    look_ = normaliseFast(look_);
    if (dot(look_, forward_) < HEART_COS60) {
        if (dot(look_, up_) > 0.f) {
            look_ = look_*hMatrixFunc::rotate(HEART_COS60-dot(look_, forward_), camRight);
        } else {
            look_ = look_*hMatrixFunc::rotate(dot(look_, forward_)-HEART_COS60, camRight);
        }
    }

    forward_ = forward_*hMatrixFunc::rotate(yaw, up_);

    movement =  scale(camRight, pad_->GetAxis(HEART_PAD_LEFT_STICKX).anologueVal_*moveSpeed_);
    movement += scale(look_, pad_->GetAxis(HEART_PAD_LEFT_STICKY).anologueVal_*moveSpeed_);

    movement += (pad_->GetButton(HEART_PAD_DPAD_UP).buttonVal_ ? scale(up_, moveSpeed_) : hVec3Func::zeroVector());
    movement += (pad_->GetButton(HEART_PAD_DPAD_DOWN).buttonVal_ ? scale(up_, -moveSpeed_) : hVec3Func::zeroVector());

    position_ += movement;

    viewMatrix_ = hMatrixFunc::LookAt(position_, position_+look_, up_);
#endif
}
