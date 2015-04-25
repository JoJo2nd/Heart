/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef hcClock_h__
#define hcClock_h__

#include "base/hTypes.h"

namespace Heart
{
    class hTimer;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    namespace hClock {
        
        HEART_EXPORT void HEART_API initialise();
        HEART_EXPORT hFloat HEART_API elapsed();
        HEART_EXPORT hUint64 HEART_API elapsedMilli();
        HEART_EXPORT hUint64 HEART_API elapsedNano();
        HEART_EXPORT hUint64 HEART_API deltams();
        HEART_EXPORT hFloat HEART_API fdeltams();
        HEART_EXPORT hFloat HEART_API delta();
        HEART_EXPORT hUint32 HEART_API hours();
        HEART_EXPORT hUint32 HEART_API mins();
        HEART_EXPORT hUint32 HEART_API secs();
        HEART_EXPORT void HEART_API update();
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class HEART_CLASS_EXPORT hTimer
    {
    public:

        hTimer();
        void    reset();
        void    setPause(hBool val);
        hBool   getPaused() const { return pauseStack_ != 0; }
        hInt64   elaspedPause() const;
        hInt64   elapsedSec() const;
        hInt64   elapsedMilliSec() const;

    private:

        hInt64 begin_;
        hInt64 lastPause_;
        hInt64 pauseTotal_;
        hUint  pauseStack_;
    };
}// namespace Heart

#endif // hcClock_h__