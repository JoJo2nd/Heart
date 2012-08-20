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

#ifdef HEART_PLAT_WINDOWS

#include <intrin.h>
#pragma intrinsic(__rdtsc)

#endif // HEART_PLAT_WINDOWS

//#define HEART_PROFILE_QUICK_OFF

namespace Heart
{
    struct HEARTBASE_SLIBEXPORT hProfileEntry
    {
        const hChar*    tag_;
        hUint32         microSecExclusive_;
        hUint32         microSecInclusive_;
    };

	class HEARTBASE_SLIBEXPORT hProfilerManager
	{
	public:
		hProfilerManager();
		virtual ~hProfilerManager();

        static const hUint32    s_maxProfileEntries = 512;

        hProfileEntry*          CreateEntry(const hChar* tag);
        hUint32                 GetTotalEntries() const { return entryCount_; }
        void                    CopyAndSortEntries(hProfileEntry* outEntries);
		void			        BeginFrame();
        void			        EnterScope( hProfileEntry* et );
        void			        ExitScope( hProfileEntry* et, hUint32 time );
        static hUint64          GetProcessorCycles() { return processorSpeed_; }
        static hUint64          GetProcessorCyclesToMilli() { return GetProcessorCycles()/1000; }
        static hUint64          GetProcessorCyclesToMicro() { return GetProcessorCycles()/100000; }
	private:

        hUint32                 inclusiveTime_;
        hUint32                 entryCount_;
        hProfileEntry           entries_[s_maxProfileEntries];
        hUint32                 stackTop_;
        hProfileEntry*          stack_[s_maxProfileEntries];
        static hUint64          processorSpeed_;
	};

    HEARTBASE_SLIBEXPORT
	extern hProfilerManager* g_ProfilerManager_;

	struct HEARTBASE_SLIBEXPORT hProfileScope
	{
		hProfileScope( hProfileEntry* pIdx )
            : entry_(pIdx)
        {
#ifndef HEART_PROFILE_QUICK_OFF
#if defined HEART_PLAT_WINDOWS
            cycles_ = __rdtsc();
#else
			hClock::BeginTimer( timer_ );
#endif
            g_ProfilerManager_->EnterScope( entry_ );
#endif // HEART_PROFILE_QUICK_OFF
		}

		~hProfileScope()
		{
#ifndef HEART_PROFILE_QUICK_OFF
#if defined HEART_PLAT_WINDOWS
            cycles_ = __rdtsc() - cycles_;
            cycles_ = cycles_/g_ProfilerManager_->GetProcessorCyclesToMicro();
            g_ProfilerManager_->ExitScope( entry_, (hUint32)cycles_ );
#else
			hClock::EndTimer( timer_ );
            g_ProfilerManager_->ExitScope( entry_, timer_.ElapsedmS() );
#endif
#endif // HEART_PROFILE_QUICK_OFF
		}

        hProfileEntry*           entry_;
#if defined HEART_PLAT_WINDOWS
        hUint64                 cycles_;
#else
		hTimer					timer_;
#endif
	};
}

#ifdef HEART_DO_PROFILE
    #define HEART_PROFILE_FUNC() static Heart::hProfileEntry* __hpfEntry = Heart::g_ProfilerManager_->CreateEntry(__FUNCTION__); Heart::hProfileScope __xxx__(__hpfEntry)
    #define HEART_PROFILE_SCOPE(StrTag) static Heart::hProfileEntry* __hpfEntry = Heart::g_ProfilerManager_->CreateEntry(StrTag); Heart::hProfileScope __xxxScope__ (__hpfEntry)
#else
    #define HEART_PROFILE_FUNC()
    #define HEART_PROFILE_SCOPE(StrTag)
#endif


#endif // PROFILER_H__