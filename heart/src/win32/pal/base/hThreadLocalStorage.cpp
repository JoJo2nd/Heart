/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#include "base/hThreadLocalStorage.h"
#include "base/hMutex.h"
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

HEART_EXPORT hSize_t HEART_API createKey(hKeyDestructor destructor) {
    tlsMutex.Lock();
    hSize_t tlsSlot = TlsAlloc();
    hTlsDestructor dtor;
    dtor.tlsSlot_ = tlsSlot;
    dtor.tlsDestructor_ = destructor;
    tlsDestructorArray.push_back(dtor);
    tlsMutex.Unlock();
    return tlsSlot;
}

HEART_EXPORT void HEART_API deleteKey(hSize_t key) {
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

HEART_EXPORT void HEART_API setKeyValue(hSize_t key, void* value) {
    TlsSetValue((DWORD)key, value);
}

HEART_EXPORT void* HEART_API getKeyValue(hSize_t key) {
    return TlsGetValue((DWORD)key);
}

HEART_EXPORT void HEART_API threadExit() {
    for (auto i=tlsDestructorArray.cbegin(), n=tlsDestructorArray.cend(); i!=n; ++i) {
        if (i->tlsDestructor_) {
            i->tlsDestructor_(getKeyValue(i->tlsSlot_));
        }
    }
}

}
}