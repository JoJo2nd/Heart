/********************************************************************

	filename: 	hClock.cpp	
	
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

#include "base/hClock.h"
#include "base/hProfiler.h"
#include <time.h>

namespace Heart {
namespace hClock {
namespace {
	struct timespec clockRes_;
	struct timespec timeStart_;
	hUint64 clockDiv_; // divider to get to milliseconds
	hUint64 tick_;
	hUint64 lastTick_;
}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hFloat elapsed() {
		return elapsedMilli() / 1000.f;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint64 elapsedMilli()	{
		return elapsedNano() / clockDiv_;
	}

	hUint64 elapsedNano() {
		struct timespec time_now;
		clock_gettime(CLOCK_MONOTONIC, &time_now);
		return timeStart_.tv_nsec - time_now.tv_nsec / clockRes_.tv_nsec;	
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint64			deltams() {
		return tick_/clockDiv_;
	}
	
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hFloat			fdeltams() {
		return tick_ / (hFloat)clockDiv_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hFloat			delta()	{
		return (tick_ / clockDiv_) / 1000.f;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32			hours()	{
		return ( ( (hUint32)elapsed() / 60 ) / 60 );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32			mins() {
		return ( (hUint32)elapsed() / 60 ) - ( hours() * 60 );
	}
	
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32			secs() {
		return (hUint32)elapsed() - ( mins() * 60 );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void update() {
		tick_ = elapsedNano() - lastTick_;
		lastTick_ = elapsedNano();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void initialise()
	{
		struct timespec res;
		hInt rc;

		rc = clock_getres(CLOCK_MONOTONIC, &clockRes_);
		hcAssert(!rc); // !!JM todo: handle other cases
		clock_gettime(CLOCK_MONOTONIC, &timeStart_);
		clockDiv_ = clockRes_.tv_nsec / 1000000;
		lastTick_ = elapsedNano();
	}

}// namespace hClock
}// namespace Heart