/********************************************************************

	filename: 	hRealTimeProfiler.cpp	
	
	Copyright (c) 17:8:2012 James Moran
	
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
/*
    Heart::hProfileEntry hRTProfilerMenu::s_entryCopies[hProfilerManager::s_maxProfileEntries];

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRTProfilerMenu::EndFrameUpdate()
    {
        hFloat cpuTime = GetProfiler()->GetFrameTime();
        hFloat gpuTime = renderer_->GetLastGPUTime();
        hChar labelStr[4096];
        hStrPrintf(labelStr, 2048, "CPU: %f - GPU: %f", cpuTime, gpuTime );
        cpugpuText_->SetText(labelStr);
        cpugpuText_->SizeToContents();


#if defined (HEART_DO_PROFILE)
        const hUint32 maxe = GetProfiler()->GetTotalEntries();
        GetProfiler()->CopyAndSortEntries(s_entryCopies);
        for (hUint32 i = 0; i < s_maxCPUtimes && i < maxe; ++i)
        {
            hStrPrintf(labelStr, 4096, "%u) %u microsecs- %s", i+1, s_entryCopies[i].microSecInclusive_-s_entryCopies[i].microSecExclusive_, s_entryCopies[i].tag_);
            cpuTimesText_[i]->SetText(labelStr);
            cpuTimesText_[i]->SizeToContents();
        }
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRTProfilerMenu::hRTProfilerMenu( Gwen::Controls::Base* parent, hRenderer* renderer ) 
        : hDebugMenuBase(parent)
        , renderer_(renderer)
    {
        cpugpuText_ = hNEW(GetDebugHeap(), Gwen::Controls::Label)(this);
        cpugpuText_->SetPos(10, 10);

        headerText_ = hNEW(GetDebugHeap(), Gwen::Controls::Label)(this);
        headerText_->SetPos(10, 30);
#if !defined (HEART_DO_PROFILE)
        headerText_->SetText("Profiling Disabled:");
#else
        headerText_->SetText("Top CPU Times (microsecs):");
#endif
        headerText_->SizeToContents();

        for (hUint32 i = 0; i < s_maxCPUtimes; ++i)
        {
            cpuTimesText_[i] = hNEW(GetDebugHeap(), Gwen::Controls::Label)(this);
            cpuTimesText_[i]->SetPos(10, 60+(30*i));
            cpuTimesText_[i]->SetText("---");

        }

        SetClosable(hTrue);
        SetDeleteOnClose(false);
        SetTitle("Real-Time Profile");

        //SizeToChildren(); Doesn't work?
        SetMinimumSize(Gwen::Point(100, 100));
        SetSize(400, 400);
        SetPos(50, 400);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRTProfilerMenu::~hRTProfilerMenu()
    {

    }
*/

}