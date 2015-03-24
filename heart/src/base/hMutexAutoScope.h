/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef hcMutex_h__
#define hcMutex_h__

#include "base/hMutex.h"

namespace Heart {
    class hMutexAutoScope {
    public:
        hMutexAutoScope( hMutex* mtx )
            : mtx_(mtx)
        {
            mtx_->Lock();
        }
        ~hMutexAutoScope()
        {
            mtx_->Unlock();
        }
    private:
        
        hMutexAutoScope( const hMutexAutoScope& rhs );

        hMutex*     mtx_;
    };

}
#endif//