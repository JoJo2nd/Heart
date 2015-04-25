/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "events/hEventManager.h"
#include "base/hThread.h"

namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hPublisherContext::initialise(hUint bufferSize)
    {
        hcAssert(bufferSize > 0);

        signalBufferSize_ = bufferSize;
        activeBuffer_ = 0;
        threadID_ = Heart::Device::GetCurrentThreadID();
        sbPos_ = signalBuffer_[0] = (hUint8*)hMalloc(signalBufferSize_*2);
        signalBuffer_[1] = signalBuffer_[0]+signalBufferSize_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void* hPublisherContext::pushSignal(hUint sizeneeded, hDispatchDelegate proc)
    {
        //Need thread id check
        hcAssertMsg(threadID_ != NULL, "hPublisherContext not initialised");
        hcAssertMsg(threadID_ == Heart::Device::GetCurrentThreadID(), "hPublisherContext must only be accessed from one thread");

        hcAssertMsg(signalBufferSize_-((hSize_t)sbPos_ - (hSize_t)signalBuffer_[activeBuffer_]) >= sizeneeded, "Run out of space in deferred dispatch buffer");
        if (signalBufferSize_-((hSize_t)sbPos_ - (hSize_t)signalBuffer_[activeBuffer_]) < sizeneeded) {
            return NULL;
        }
        
        SignalHeader* sh = (SignalHeader*)sbPos_;
        sh->delegate_ = proc;
        sh->dataSize_ = sizeneeded+sizeof(SignalHeader);
        void* ret = sbPos_+sizeof(SignalHeader);
        sbPos_+= sh->dataSize_;
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hPublisherContext::dispatch()
    {
        hUint8* sbEnd = sbPos_;
        //Swap buffers
        activeBuffer_=activeBuffer_^1;
        sbPos_ = signalBuffer_[activeBuffer_];
        //process the swapped buffer, allows new events to be pushed
        for (hUint8* ptr=signalBuffer_[activeBuffer_^1]; ptr < sbEnd; ptr += ((SignalHeader*)ptr)->dataSize_) {
            SignalHeader* sh=(SignalHeader*)ptr;
            sh->delegate_((void*)(sh+1), sh->dataSize_);
        }        
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hPublisherContext::updateDispatch(hUint maxIterations /*= 10*/)
    {
        for (hUint i=0; i<maxIterations && sbPos_ != signalBuffer_[activeBuffer_]; ++i) {
            dispatch();
        }
    }
}