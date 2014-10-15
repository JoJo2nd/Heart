/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef EVENT_H__
#define EVENT_H__

#include "base/hTypes.h"
#include "base/hFunctor.h"
#include "base/hArray.h"

namespace Heart
{
    /************************************************************************
    * Different system using publisher/subscriber style pattern
    ************************************************************************/
    hFUNCTOR_TYPEDEF(void (*)(void* /*data*/, hUint /*size*/), hDispatchDelegate);

    class  hPublisherContext
    {
    public:
        hPublisherContext() 
            : threadID_(nullptr)
            , activeBuffer_(0)
            , sbPos_(nullptr)
            , signalBufferSize_(0)
        {
            signalBuffer_[0] = signalBuffer_[1] = nullptr;
        }
        ~hPublisherContext()
        {
            delete[] signalBuffer_[0];
            signalBuffer_[1] = nullptr;
            sbPos_ = nullptr;
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

#include "hPublisher_gen.inl"

}
#endif // EVENT_H__