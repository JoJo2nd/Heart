/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hClock.h"
#include "base/hProfiler.h"
#include <winsock2.h>
#include <windows.h>

namespace Heart {
namespace hClock {

	hInt64			Time_;
	hInt64			TimeMS_;
	hInt64			LastTime_;
	hUint32			TickMS_;
	hFloat			TickS_;
	hInt64			StartTime_;
	hInt64			Freq_;
	hInt64			FreqMicro_;

	HEART_EXPORT hFloat HEART_API elapsed() {
		return static_cast< hFloat >( (Time_ - StartTime_) / 1000.0f );
	}

	HEART_EXPORT hUint64 HEART_API elapsedMilli() {
		return Time_ - StartTime_;
	}

	HEART_EXPORT hUint64 HEART_API deltams()
	{
		return TickMS_;
	}

	HEART_EXPORT hFloat HEART_API fdeltams()
	{
		return static_cast< hFloat >( TickMS_ );
	}

	HEART_EXPORT hFloat HEART_API delta()
	{
		return TickS_;
	}

	HEART_EXPORT hUint32 HEART_API hours()
	{
		return ( ( (hUint32)elapsed() / 60 ) / 60 );
	}

	HEART_EXPORT hUint32 HEART_API mins()
	{
		return ( (hUint32)elapsed() / 60 ) - ( hours() * 60 );
	}

	HEART_EXPORT hUint32 HEART_API secs()
	{
		return (hUint32)elapsed() - ( mins() * 60 );
	}

	HEART_EXPORT void HEART_API update()
	{
        HEART_PROFILE_FUNC();
		if ( Freq_ != 0 )
		{
			LastTime_ = Time_;

			QueryPerformanceCounter( reinterpret_cast< LARGE_INTEGER* >( &Time_ ) );

			Time_ /= Freq_;
			TickMS_ = (hUint32)( Time_ - LastTime_ );
			TickS_ = TickMS_ / 1000.0f;
			TimeMS_ = Time_;
		}
	}

	HEART_EXPORT void HEART_API initialise()
	{
		Time_ = 0;
		LastTime_ = 0;
		TickS_ = 0.0f;
		TickMS_ = 0;

		QueryPerformanceFrequency( reinterpret_cast< LARGE_INTEGER*>( &Freq_ ) );

        FreqMicro_	= Freq_/1000000;
		Freq_		/= 1000;

		QueryPerformanceCounter( reinterpret_cast< LARGE_INTEGER* >( &StartTime_ ) );

		StartTime_ /= Freq_;
	}

}// namespace hClock

hTimer::hTimer() {
    reset();
}

void hTimer::reset() {
    begin_ = hClock::elapsedMilli();
    pauseStack_ = 0;
    lastPause_ = begin_;
    pauseTotal_ = 0;
}

void hTimer::setPause(hBool val) {
    if (val) {
        ++pauseStack_;
        if (pauseStack_ == 1) {
            lastPause_ = hClock::elapsedMilli();
        }
    }
    else if (!val && pauseStack_ > 0) {
        pauseTotal_ += hClock::elapsedMilli() - lastPause_;
    }
}

hInt64 hTimer::elaspedPause() const {
    hInt64 current = hClock::elapsedMilli() - lastPause_;
    return getPaused() ? pauseTotal_ + current : pauseTotal_;
}

hInt64 hTimer::elapsedSec() const {
    return elapsedMilliSec() / 1000;
}

hInt64 hTimer::elapsedMilliSec() const {
    return (hClock::elapsedMilli() - begin_) - elaspedPause();
}

}// namespace Heart