/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "hSprite2D.h"

namespace Heart {

hRegisterObjectType(hSprite2D, Heart::hSprite2D, Heart::proto::RenderSprite2D);

hBool hSprite2D::serialiseObject(Heart::proto::RenderSprite2D* obj) const {
    return hTrue;
}

hBool hSprite2D::deserialiseObject(Heart::proto::RenderSprite2D* obj) {
    return hTrue;
}

hBool hSprite2D::linkObject() {
    return hTrue;
}

}