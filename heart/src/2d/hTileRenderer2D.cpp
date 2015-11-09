/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "2d/hRenderPlane2D.h"
#include "2d/hSprite2D.h"
#include "2d/hDynamicTileSet2D.h"
#include "components/hEntity.h"
#include "components/hEntityFactory.h"
#include "components/hObjectFactory.h"

namespace Heart {
namespace hTileRenderer2D {

hBool registerComponents() {
    Heart::hEntityFactory::hComponentMgt c;
    c.object_def = hObjectFactory::getObjectDefinition(hRenderPlane2D::getTypeNameStatic());
    c.construct = [](hEntity* owner) -> hEntityComponent* { return new hRenderPlane2D(); };
    c.destruct = [](hEntityComponent* ptr) { delete ptr; };
    hEntityFactory::registerComponentManagement(c);

    c.object_def = hObjectFactory::getObjectDefinition(hSprite2D::getTypeNameStatic());
    c.construct = [](hEntity* owner) -> hEntityComponent* { return new hSprite2D(); };
    c.destruct = [](hEntityComponent* ptr) { delete ptr; };
    hEntityFactory::registerComponentManagement(c);

    c.object_def = hObjectFactory::getObjectDefinition(hDynamicTileSet2D::getTypeNameStatic());
    c.construct = [](hEntity* owner) -> hEntityComponent* { return new hDynamicTileSet2D(); };
    c.destruct = [](hEntityComponent* ptr) { delete ptr; };
    hEntityFactory::registerComponentManagement(c);

    return hTrue;
}

}
}
