/********************************************************************

    filename: 	hClock.h	
    
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