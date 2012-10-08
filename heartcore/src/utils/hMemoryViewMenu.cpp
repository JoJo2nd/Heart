/********************************************************************

	filename: 	hMemoryViewMenu.cpp	
	
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

namespace Heart
{

#ifdef HEART_DEBUG
    hMemoryViewMenu::HeapLabelArray* hMemoryViewMenu::heapArray_ = NULL;
#endif

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMemoryViewMenu::hMemoryViewMenu( Gwen::Controls::Base* parent)
        : hDebugMenuBase(parent)
    {
//         generalHeapTxt_ = hNEW(GetDebugHeap(), Gwen::Controls::Label)(this);
//         generalHeapTxt_->SetPos(10, 10);
// 
//         debugHeapTxt_ = hNEW(GetDebugHeap(), Gwen::Controls::Label)(this);
//         debugHeapTxt_->SetPos(10, 30);

        SetClosable(hTrue);
        SetDeleteOnClose(false);
        SetTitle("Memory View");

        //SizeToChildren(); Doesn't work?
        SetMinimumSize(Gwen::Point(900, 100));
        SetSize(400, 100);
        SetPos(0, 0);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMemoryViewMenu::~hMemoryViewMenu()
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMemoryViewMenu::PreRenderUpdate()
    {
        hChar labelStr[4096];
        hMemoryHeapBase::HeapInfo info;
        hFloat fMB;
//         hMemoryHeapBase::HeapInfo info = GetGlobalHeap()->usage();
//         hFloat fMB = (hFloat)info.totalBytesAllocated_/(1024.f*1024.f);
//         
//         
//         hStrPrintf(labelStr, 2048, 
//             "Global Heap: %fMB (%u bytes, %u peak, %u non-mmap, %u mmap, %u free chunks, %u free space)", 
//             fMB, 
//             info.totalBytesAllocated_,
//             info.peakBytesReserved_,
//             info.exData_.arena,
//             info.exData_.hblkhd,
//             info.exData_.ordblks,
//             info.exData_.fordblks);
//         generalHeapTxt_->SetText(labelStr);
//         generalHeapTxt_->SizeToContents();
// 
//         info = GetDebugHeap()->usage();
//         fMB = (hFloat)info.totalBytesAllocated_/(1024.f*1024.f);
//         hStrPrintf(labelStr, 2048, "Debug Heap: %fMB (%u bytes, %u peak, %u non-mmap, %u mmap, %u free chunks, %u free space)", 
//             fMB, 
//             info.totalBytesAllocated_,
//             info.peakBytesReserved_,
//             info.exData_.arena,
//             info.exData_.hblkhd,
//             info.exData_.ordblks,
//             info.exData_.fordblks);
//         debugHeapTxt_->SetText(labelStr);
//         debugHeapTxt_->SizeToContents();

        for(hUint32 i = 0; i < (*heapArray_).GetSize(); ++i)
        {
            if ((*heapArray_)[i].txt == NULL)
            {
                (*heapArray_)[i].txt = hNEW(GetDebugHeap(), Gwen::Controls::Label)(this);
            }

            info = (*heapArray_)[i].heap->usage();
            fMB = (hFloat)info.totalBytesAllocated_/(1024.f*1024.f);
            hStrPrintf(labelStr, 2048, "%s: %fMB (%u bytes, %u peak, %u non-mmap, %u mmap, %u free chunks, %u free space)", 
                (*heapArray_)[i].heap->getHeapName(),
                fMB, 
                info.totalBytesAllocated_,
                info.peakBytesReserved_,
                info.exData_.arena,
                info.exData_.hblkhd,
                info.exData_.ordblks,
                info.exData_.fordblks);
            (*heapArray_)[i].txt->SetText(labelStr);
            (*heapArray_)[i].txt->SetPos(10, 10+(i*20));
            (*heapArray_)[i].txt->SizeToContents();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMemoryViewMenu::RegisterMemoryHeap( hMemoryHeapBase* heap )
    {
#ifdef HEART_DEBUG

        if (heapArray_ == NULL)
        {
            heapArray_ = hNEW(GetDebugHeap(), hMemoryViewMenu::HeapLabelArray)(GetDebugHeap());
        }

        ExtraHeap exh;
        exh.heap    = heap;
        exh.txt     = NULL;
        (*heapArray_).PushBack(exh);
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMemoryViewMenu::UnregisterMemoryHeap( hMemoryHeapBase* heap )
    {
#ifdef HEART_DEBUG
        for(hUint32 i = 0; i < (*heapArray_).GetSize(); ++i)
        {
            if (heap == (*heapArray_)[i].heap)
            {
                hDELETE_SAFE(GetDebugHeap(), (*heapArray_)[i].txt);
                (*heapArray_)[i] = (*heapArray_)[(*heapArray_).GetSize()-1];
                (*heapArray_).Resize((*heapArray_).GetSize()-1);
                return;
            }
        }
#endif
    }

}