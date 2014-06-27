/********************************************************************

    filename:   osfuncs.cpp  
    
    Copyright (c) 22:12:2013 James Moran
    
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