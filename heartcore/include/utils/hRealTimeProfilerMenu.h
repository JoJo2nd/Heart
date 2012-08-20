/********************************************************************

	filename: 	hRealTimeProfilerMenu.h	
	
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
#ifndef HREALTIMEPROFILERMENU_H__
#define HREALTIMEPROFILERMENU_H__

namespace Heart
{
    class hRTProfilerMenu : public Gwen::Controls::WindowControl
    {
    public:
        hRTProfilerMenu(Gwen::Controls::Base* parent);
        ~hRTProfilerMenu();

        void                EndFrameUpdate(hFloat cpuTime, hFloat gpuTime);
    
    private:

        static const hUint32    s_maxCPUtimes = 25;

        static hProfileEntry    s_entryCopies[hProfilerManager::s_maxProfileEntries];
        Gwen::Controls::Label*  cpugpuText_;
        Gwen::Controls::Label*  headerText_;
        Gwen::Controls::Label*  cpuTimesText_[hProfilerManager::s_maxProfileEntries];
    };
}

#endif // HREALTIMEPROFILERMENU_H__