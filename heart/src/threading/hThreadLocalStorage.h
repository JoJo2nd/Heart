/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/


#include "base/hTypes.h"

namespace Heart{
namespace TLS {

typedef void (*hKeyDestructor)(void* key_value);

hSize_t createKey(hKeyDestructor destructor);
void    deleteKey(hSize_t key);
void	setKeyValue(hSize_t key, void* value);
void*   getKeyValue(hSize_t key);

#if defined HEART_PLAT_WINDOWS
    void    threadExit();
#endif

}	
}    