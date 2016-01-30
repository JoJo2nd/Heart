/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "hTechniques.h"
#include "base/hMutex.h"

namespace Heart {
namespace hTechniques {
namespace {
    hMutex techniqueMtx;
    hUint techniqueCount = 0;
    hStringID techniqueNames[31];
}

void registerTechnique(hStringID technique_name) {
    for (hUint i = 0; i < techniqueCount; ++i) {
        if (techniqueNames[i] == technique_name) return;
    }
    techniqueNames[techniqueCount++] = technique_name;
}

hUint32 getTechniqueFlag(hStringID technique_name) {
    for (hUint i = 0; i < techniqueCount; ++i) {
        if (techniqueNames[i] == technique_name) return (1 << i);
    }
    return 0;
}

hUint32 getTechniqueFlags(hStringID* technqiue_names, hUint count) {
    hUint32 r = 0;
    for (hUint j = 0; j < count; ++j) {
        for (hUint i = 0; i < techniqueCount; ++i) {
            if (techniqueNames[i] == technqiue_names[j]) {
                r |= (1 << i);
                break;
            }
        }
    }
    return r;
}

}
}