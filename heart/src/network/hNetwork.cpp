/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "network/hNetwork.h"
//#include <winsock2.h>
//#include <windows.h>
#include "enet/enet.h"
#include "base/hTypes.h"
#include "base/hMemory.h"

namespace Heart {
namespace hNetwork {
    static void* ENET_CALLBACK henet_malloc(size_t size) {
        return hMalloc(size);
    }
    static void ENET_CALLBACK henet_free(void* ptr) {
        hFree(ptr);
    }
    static void ENET_CALLBACK henet_nomem() {
        hcAssertFailMsg("enet No Memory");
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void initialise() {
        ENetCallbacks callbacks = {henet_malloc, henet_free, henet_nomem};
        enet_initialize_with_callbacks(ENET_VERSION, &callbacks);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void shutdown() {
        enet_deinitialize();
    }
}
}