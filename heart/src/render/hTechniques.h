/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "base/hStringID.h"

namespace Heart {
namespace hTechniques {
void registerTechnique(hStringID technique_name);
hUint32 getTechniqueFlag(hStringID technique_name);
hUint32 getTechniqueFlags(hStringID* technqiue_names, hUint count);
}
}