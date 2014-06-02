/********************************************************************

	filename: 	hProfiler.h	
	
	Copyright (c) 19:5:2010 James Moran
	
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

#ifndef PROFILER_H__
#define PROFILER_H__

#include "base/hTypes.h"
#include "base/hSysCalls.h"
#include "base/hClock.h"

namespace Heart
{
    struct hProfileEntry
    {
        const hChar*    tag_;
        hUint64         microSecExclusive_;
        hUint64         microSecInclusive_;
    };

	class hProfilerManager
	{
	public:
		hProfilerManager();
		virtual ~hProfilerManager();

        static const hUint32    s_maxProfileEntries = 512;

        hProfileEntry*          CreateEntry(const hChar* tag);
        hUint64                 GetTotalEntries() const { return entryCount_; }
        void                    CopyAndSortEntries(hProfileEntry* outEntries);
		void			        BeginFrame();
        void			        EnterScope( hProfileEntry* et );
        void			        ExitScope( hProfileEntry* et, hUint64 time );
        void                    SetFrameTime(hFloat val) { frameTime_ = val; }
        hFloat                  GetFrameTime() const { return frameTime_; }
        static hUint64          GetProcessorCycles() { return hSysCall::GetProcessorSpeed(); }
        static hUint64          GetProcessorCyclesToMilli() { return GetProcessorCycles()/1000; }
        static hUint64          GetProcessorCyclesToMicro() { return GetProcessorCycles()/100000; }
	private:

        hUint64                 inclusiveTime_;
        hUint64                 entryCount_;
        hProfileEntry           entries_[s_maxProfileEntries];
        hUint64                 stackTop_;
        hProfileEntry*          stack_[s_maxProfileEntries];
        hFloat                  frameTime_;
	};

    hProfilerManager* GetProfiler();

	struct hProfileScope
	{
		hProfileScope(hProfileEntry* pIdx)
            : entry_(pIdx)
        {
            GetProfiler()->EnterScope( entry_ );
		}

		~hProfileScope()
		{
            GetProfiler()->ExitScope( entry_, timer_.elapsedMilliSec() );
		}

        hProfileEntry*  entry_;
		hTimer			timer_;
	};
}

#ifdef HEART_DO_PROFILE
    #define HEART_PROFILE_FUNC() static Heart::hProfileEntry* __hpfEntry = Heart::GetProfiler()->CreateEntry(__FUNCTION__); Heart::hProfileScope __xxx__(__hpfEntry)
    #define HEART_PROFILE_SCOPE(StrTag) static Heart::hProfileEntry* __hpfEntry = Heart::GetProfiler()->CreateEntry(StrTag); Heart::hProfileScope __xxxScope__ (__hpfEntry)
#else
    #define HEART_PROFILE_FUNC()
    #define HEART_PROFILE_SCOPE(StrTag)
#endif


#endif // PROFILER_H__