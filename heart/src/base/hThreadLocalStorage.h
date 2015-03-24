/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/


#include "base/hTypes.h"

namespace Heart{
namespace TLS {

typedef void (HEART_API *hKeyDestructor)(void* key_value);

HEART_EXPORT hSize_t HEART_API createKey(hKeyDestructor destructor);
HEART_EXPORT void    HEART_API deleteKey(hSize_t key);
HEART_EXPORT void    HEART_API setKeyValue(hSize_t key, void* value);
HEART_EXPORT void*   HEART_API getKeyValue(hSize_t key);

#if defined HEART_PLAT_WINDOWS
    HEART_EXPORT void HEART_API threadExit();
#endif

}	
}    