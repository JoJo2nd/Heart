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

namespace Heart
{
    class hTimer;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    namespace hClock
    {
        HEART_DLLEXPORT
        void initialise();
        HEART_DLLEXPORT
        hFloat elapsed();
        HEART_DLLEXPORT
        hInt64 elapsedMS();
        HEART_DLLEXPORT
        hUint32 deltams();
        HEART_DLLEXPORT
        hFloat fdeltams();
        HEART_DLLEXPORT
        hFloat delta();
        HEART_DLLEXPORT
        hUint32 hours();
        HEART_DLLEXPORT
        hUint32 mins();
        HEART_DLLEXPORT
        hUint32 secs();
        HEART_DLLEXPORT
        void update();
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class HEART_DLLEXPORT hTimer
    {
    public:

        hTimer() {
            reset();
        }
        void    reset() {
            begin_=hClock::elapsedMS();
            pauseStack_=0;
            lastPause_=begin_;
            pauseTotal_=0;
        }
        void    setPause(hBool val) {
            if (val) {
                ++pauseStack_;
                if (pauseStack_==1) {
                    lastPause_=hClock::elapsedMS();
                }
            } else if (!val && pauseStack_ > 0) {
                pauseTotal_+=hClock::elapsedMS()-lastPause_;
            }
        }
        hBool   getPaused() const { return pauseStack_ != 0; }
        hInt64   elaspedPause() const { 
            hInt64 current=hClock::elapsedMS()-lastPause_;
            return getPaused() ? pauseTotal_+current : pauseTotal_;
        }
        hInt64   elapsedSec() const { return elapsedMilliSec()/1000; }
        hInt64   elapsedMilliSec() const { return (hClock::elapsedMS()-begin_) - elaspedPause(); }

    private:

        hInt64 begin_;
        hInt64 lastPause_;
        hInt64 pauseTotal_;
        hUint  pauseStack_;
    };
}// namespace Heart

#endif // hcClock_h__