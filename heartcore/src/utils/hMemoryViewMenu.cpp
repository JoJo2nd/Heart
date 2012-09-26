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

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMemoryViewMenu::hMemoryViewMenu( Gwen::Controls::Base* parent)
        : hDebugMenuBase(parent)
    {
        generalHeapTxt_ = hNEW(GetDebugHeap(), Gwen::Controls::Label)(this);
        generalHeapTxt_->SetPos(10, 10);

        debugHeapTxt_ = hNEW(GetDebugHeap(), Gwen::Controls::Label)(this);
        debugHeapTxt_->SetPos(10, 30);

        SetClosable(hTrue);
        SetDeleteOnClose(false);
        SetTitle("Memory View");

        //SizeToChildren(); Doesn't work?
        SetMinimumSize(Gwen::Point(100, 100));
        SetSize(400, 100);
        SetPos(400, 100);
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
        hFloat fMB = (hFloat)GetGlobalHeap()->bytesAllocated()/(1024.f*1024.f);
        
        hStrPrintf(labelStr, 2048, "Global Heap Used: %fMB (%u bytes)", fMB, GetGlobalHeap()->bytesAllocated());
        generalHeapTxt_->SetText(labelStr);
        generalHeapTxt_->SizeToContents();

        fMB = (hFloat)GetDebugHeap()->bytesAllocated()/(1024.f*1024.f);
        hStrPrintf(labelStr, 2048, "Debug Heap Used: %fMB (%u bytes)", fMB, GetDebugHeap()->bytesAllocated());
        debugHeapTxt_->SetText(labelStr);
        debugHeapTxt_->SizeToContents();
    }

}