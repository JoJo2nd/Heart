/********************************************************************

    filename: 	hEventManager.h	
    
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

#ifndef EVENT_H__
#define EVENT_H__

namespace Heart
{
    /************************************************************************
    * Different system using publisher/subscriber style pattern
    ************************************************************************/
    hFUNCTOR_TYPEDEF(void (*)(void*/*data*/, hUint/*size*/), hDispatchDelegate);

    class HEART_DLLEXPORT hPublisherContext
    {
    public:
        hPublisherContext() 
            : threadID_(NULL)
            , activeBuffer_(0)
            , sbPos_(NULL)
            , signalBufferSize_(0)
        {
            signalBuffer_[0] = signalBuffer_[1] = NULL;
        }
        ~hPublisherContext()
        {
            hFreeSafe(signalBuffer_[0]);
            signalBuffer_[1] = NULL;
            sbPos_ = NULL;
            signalBufferSize_ = 0;
        }
        void  initialise(hUint bufferSize);
        void* pushSignal(hUint sizeneeded, hDispatchDelegate proc);
        void  updateDispatch(hUint maxIterations = 2);

    private:

        struct SignalHeader 
        {
            hDispatchDelegate delegate_;
            hUint             dataSize_; // - not inc. size of this header
        };

        void  dispatch();

        void*   threadID_; // Heart::Device::GetCurrentThreadID()
        hUint   activeBuffer_;
        hUint8* signalBuffer_[2];
        hUint8* sbPos_;
        hUint   signalBufferSize_;
    };

#include "generated/hPuslisher_gen.h"

}
#endif // EVENT_H__