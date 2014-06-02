/********************************************************************

    filename:   hNetwork.cpp  
    
    Copyright (c) 20:9:2013 James Moran
    
    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.
    
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
    
    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
    
    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
    
    3. This notice may not be removed or altered from any source
    distribution.

*********************************************************************/

#include "network/hNetwork.h"
#include <winsock2.h>
#include <windows.h>
#include "enet/enet.h"
#include "base/hTypes.h"

namespace Heart
{
namespace hNetwork
{
    static void* ENET_CALLBACK henet_malloc(size_t size) {
        return new hByte[size];
    }
    static void ENET_CALLBACK henet_free(void* ptr) {
        delete [] ptr;
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