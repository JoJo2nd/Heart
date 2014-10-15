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
        void initialise();
        hFloat elapsed();
        hUint64 elapsedMilli();
        hUint64 elapsedNano();
        hUint64 deltams();
        hFloat fdeltams();
        hFloat delta();
        hUint32 hours();
        hUint32 mins();
        hUint32 secs();
        void update();
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class hTimer
    {
    public:

        hTimer() {
            reset();
        }
        void    reset() {
            begin_=hClock::elapsedMilli();
            pauseStack_=0;
            lastPause_=begin_;
            pauseTotal_=0;
        }
        void    setPause(hBool val) {
            if (val) {
                ++pauseStack_;
                if (pauseStack_==1) {
                    lastPause_=hClock::elapsedMilli();
                }
            } else if (!val && pauseStack_ > 0) {
                pauseTotal_+=hClock::elapsedMilli()-lastPause_;
            }
        }
        hBool   getPaused() const { return pauseStack_ != 0; }
        hInt64   elaspedPause() const { 
            hInt64 current=hClock::elapsedMilli()-lastPause_;
            return getPaused() ? pauseTotal_+current : pauseTotal_;
        }
        hInt64   elapsedSec() const { return elapsedMilliSec()/1000; }
        hInt64   elapsedMilliSec() const { return (hClock::elapsedMilli()-begin_) - elaspedPause(); }

    private:

        hInt64 begin_;
        hInt64 lastPause_;
        hInt64 pauseTotal_;
        hUint  pauseStack_;
    };
}// namespace Heart

#endif // hcClock_h__