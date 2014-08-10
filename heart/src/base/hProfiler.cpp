/********************************************************************

	filename: 	hProfiler.cpp	
	
	Copyright (c) 1:4:2012 James Moran
	
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

#include "base/hProfiler.h"
#include "base/hMemoryUtil.h"
#include <stdlib.h>

namespace Heart {
	static hProfilerManager* g_ProfilerManager_;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hProfilerManager* GetProfiler() {
        return g_ProfilerManager_;
    }

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hProfilerManager::hProfilerManager()
        : entryCount_(0)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hProfilerManager::~hProfilerManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hProfilerManager::BeginFrame()
	{
        inclusiveTime_ = 0;
        stackTop_ = 0;
        for (hUint32 i = 0; i < entryCount_; ++i)
        {
            entries_[i].microSecInclusive_ = 0;
            entries_[i].microSecExclusive_ = 0;
        }
	}
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hProfileEntry* hProfilerManager::CreateEntry( const hChar* tag )
    {
        hcAssertMsg(entryCount_ < s_maxProfileEntries, "Run out of profile entries, increase hProfileManager::MAX_ENTRIES");
        hProfileEntry* ret = entries_+entryCount_;
        entries_[entryCount_].tag_ = tag;
        entryCount_++;
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    int excluseSort(const void* lhs, const void* rhs)
    {
        return ((hProfileEntry*)lhs)->microSecInclusive_-((hProfileEntry*)lhs)->microSecExclusive_ > ((hProfileEntry*)rhs)->microSecInclusive_-((hProfileEntry*)rhs)->microSecExclusive_ ? -1 : 1;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hProfilerManager::CopyAndSortEntries( hProfileEntry* outEntries )
    {
        hMemCpy(outEntries, entries_, sizeof(hProfileEntry)*entryCount_);
        qsort(outEntries, entryCount_, sizeof(hProfileEntry), excluseSort);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hProfilerManager::EnterScope( hProfileEntry* et )
    {
        hcAssertMsg(stackTop_ < s_maxProfileEntries, "Stack overflow");
        stack_[stackTop_++] = et;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hProfilerManager::ExitScope( hProfileEntry* et, hUint64 time )
    {
        et->microSecInclusive_ += time;

        stackTop_--;
        if (stackTop_ > 0)
            stack_[stackTop_-1]->microSecExclusive_ += time;
    }
}
