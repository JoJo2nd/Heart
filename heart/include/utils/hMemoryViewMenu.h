/********************************************************************

	filename: 	hMemoryViewMenu.h	
	
	Copyright (c) 26:9:2012 James Moran
	
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
#pragma once

#ifndef HMEMORYVIEWMENU_H__
#define HMEMORYVIEWMENU_H__

namespace Heart
{
    /*
    class hMemoryViewMenu : public hDebugMenuBase
    {
    public:
        hMemoryViewMenu(Gwen::Controls::Base* parent);
        ~hMemoryViewMenu();

        void                PreRenderUpdate();
        void                EndFrameUpdate() {}

        static void         RegisterMemoryHeap(hMemoryHeapBase* heap);
        static void         UnregisterMemoryHeap(hMemoryHeapBase* heap);

    private:

        struct ExtraHeap
        {
            hMemoryHeapBase*        heap;
            Gwen::Controls::Label*  txt;
        };

        typedef hVector< ExtraHeap > HeapLabelArray;

        Gwen::Controls::Label*  generalHeapTxt_;
        Gwen::Controls::Label*  debugHeapTxt_;
#ifdef HEART_DEBUG
        static HeapLabelArray*  heapArray_;
#endif
    };
    */
}


#endif // HMEMORYVIEWMENU_H__