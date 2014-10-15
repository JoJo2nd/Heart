/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/


#include "threading/hThreadLocalStorage.h"
#include <pthread.h>

namespace Heart {
namespace TLS {    

hSize_t createKey(hKeyDestructor destructor) {
    pthread_key_t key;
    if (pthread_key_create(&key, destructor)) {
        return 0;
    }
    return (hSize_t)key;
}

void deleteKey(hSize_t key) {
    pthread_key_delete(key);
}

void    setKeyValue(hSize_t key, void* value) {
    pthread_setspecific(key, value);
}

void*   getKeyValue(hSize_t key) {
    return pthread_getspecific(key);
}

void threadExit() {}

}
}
