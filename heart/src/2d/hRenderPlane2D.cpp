/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "hRenderPlane2D.h"

namespace Heart {

hRegisterObjectType(hRenderPlane2D, Heart::hRenderPlane2D, Heart::proto::RenderPlane2D);

hBool hRenderPlane2D::serialiseObject(Heart::proto::RenderPlane2D* obj) const {
    return hTrue;
}

hBool hRenderPlane2D::deserialiseObject(Heart::proto::RenderPlane2D* obj) {
    return hTrue;
}

hBool hRenderPlane2D::linkObject() {
    return hTrue;
}

}