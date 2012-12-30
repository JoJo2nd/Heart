/********************************************************************

	filename: 	hMemoryHeapBase.cpp	
	
	Copyright (c) 7:7:2012 James Moran
	
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

// Not the cleanest way to do this but it works
extern void initialiseBaseMemoryHeaps();

namespace Heart
{
    static hMemoryHeapBase* g_globalHeap = NULL;
    static hMemoryHeapBase* g_debugHeap = NULL;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT void               HEART_API SetGlobalHeap( hMemoryHeapBase* heap )
    {
        hcAssert(g_debugHeap == NULL);
        g_globalHeap = heap;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT hMemoryHeapBase*   HEART_API GetGlobalHeap()
    {
        if(g_globalHeap == NULL) initialiseBaseMemoryHeaps();
        return g_globalHeap;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT hMemoryHeapBase*   HEART_API GetDebugHeap()
    {
        if(g_debugHeap == NULL) initialiseBaseMemoryHeaps();
        return g_debugHeap;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT void               HEART_API SetDebugHeap( hMemoryHeapBase* heap )
    {
        hcAssert(g_debugHeap == NULL);
        g_debugHeap = heap;
    }

}