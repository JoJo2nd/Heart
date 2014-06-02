/********************************************************************

    Copyright (c) James Moran
    
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

#include "pal/hThreadLocalStorage.h"
#include "pal/hMutex.h"
#include <winsock2.h>
#include <windows.h>
#include <vector>

namespace Heart {
namespace TLS {    

struct hTlsDestructor{
    hTlsDestructor() : tlsSlot_(0)
        , tlsDestructor_(nullptr)
    {}

    hSize_t         tlsSlot_;
    hKeyDestructor  tlsDestructor_;
};
hMutex tlsMutex;
std::vector< hTlsDestructor > tlsDestructorArray;

hSize_t createKey(hKeyDestructor destructor) {
    tlsMutex.Lock();
    hSize_t tlsSlot = TlsAlloc();
    hTlsDestructor dtor;
    dtor.tlsSlot_ = tlsSlot;
    dtor.tlsDestructor_ = destructor;
    tlsDestructorArray.push_back(dtor);
    tlsMutex.Unlock();
    return tlsSlot;
}

void deleteKey(hSize_t key) {
    tlsMutex.Lock();
    for (auto i=tlsDestructorArray.begin(), n=tlsDestructorArray.end(); i!=n; ++i) {
        if (i->tlsSlot_==key) {
            tlsDestructorArray.erase(i);
            break;
        }
    }
    TlsFree((DWORD)key);
    tlsMutex.Unlock();
}

void    setKeyValue(hSize_t key, void* value) {
    TlsSetValue((DWORD)key, value);
}

void*   getKeyValue(hSize_t key) {
    return TlsGetValue((DWORD)key);
}

void threadExit() {
    for (auto i=tlsDestructorArray.cbegin(), n=tlsDestructorArray.cend(); i!=n; ++i) {
        if (i->tlsDestructor_) {
            i->tlsDestructor_(getKeyValue(i->tlsSlot_));
        }
    }
}

}
}