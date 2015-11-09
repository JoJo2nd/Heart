/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "hDynamicTileSet2D.h"

namespace Heart {

hRegisterObjectType(hDynamicTileSet2D, Heart::hDynamicTileSet2D, Heart::proto::DynamicTileSet2D);

hBool hDynamicTileSet2D::serialiseObject(Heart::proto::DynamicTileSet2D* obj) const {
    return hTrue;
}

hBool hDynamicTileSet2D::deserialiseObject(Heart::proto::DynamicTileSet2D* obj) {
    return hTrue;
}

hBool hDynamicTileSet2D::linkObject() {
    return hTrue;
}

}