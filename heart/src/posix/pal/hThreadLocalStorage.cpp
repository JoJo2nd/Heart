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
