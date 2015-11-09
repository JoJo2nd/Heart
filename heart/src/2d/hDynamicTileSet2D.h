/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "core/hProtobuf.h"
#include "components/hObjectFactory.h"
#include "components/hEntity.h"
#include "resource_2d.pb.h"

namespace Heart {

class hDynamicTileSet2D : public hEntityComponent {
public:
    hObjectType(hDynamicTileSet2D, Heart::proto::DynamicTileSet2D);
};

}