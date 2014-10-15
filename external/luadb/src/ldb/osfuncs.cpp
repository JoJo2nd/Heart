/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "osfuncs.h"

#if defined (PLATFORM_WINDOWS)    
#include <windows.h>

    void ldbOSSleep(float secs) {
        Sleep((DWORD)(secs*1000.f));
    }

    ldb_uint32 ldbOSGetSysTimems() {
        return timeGetTime();
    }
#elif defined (PLATFORM_LINUX)
    #include <unistd.h>

    void ldbOSSleep(float secs) {
#if _POSIX_C_SOURCE >= 199309L
        timespec t;
        t.tv_sec = (time_t)(secs+.5f);
        t.tv_nsec = (long)(secs - t.tv_sec)*1000000000;
        nanosleep(&t, nullptr);
#else        
        sleep((uint_t)(secs+.5f));
#endif
    }

    ldb_uint32 ldbOSGetSysTimems() {
        return (clock() / CLOCKS_PER_SEC)*1000;
    }
#else
#   error ("Unknown platform")
#endif