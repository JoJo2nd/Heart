/********************************************************************

	filename: 	hResource.cpp	
	
	Copyright (c) 13:8:2011 James Moran
	
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

namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hStreamingResourceBase::QueueStreamRead( void* dstBuf, hUint32 size, hUint32 offset, hUint32* opID )
    {
        //hMutexAutoScope am( &lock_ );
        for ( hUint32 i = 0; i < MAX_READ_OPS; ++i )
        {
            if ( !readOps_[i].active_ )
            {
                readOps_[i].dstBuf_ = dstBuf;
                readOps_[i].size_ = size;
                readOps_[i].offset_ = offset;
                readOps_[i].done_ = hFalse;

                hAtomic::LWMemoryBarrier();
                readOps_[i].active_ = hTrue;

                *opID = i;
                //manager_->Post();
                return 0;
            }
        }

        *opID = ~0U;
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hStreamingResourceBase::UpdateFileOps()
    {
        //hMutexAutoScope am( &lock_ );
        for ( hUint32 i = 0; i < MAX_READ_OPS; ++i )
        {
            if ( readOps_[i].active_ && !readOps_[i].done_ )
            {
                fileStream_.Seek( readOps_[i].offset_ );
                readOps_[i].read_ = fileStream_.Read( readOps_[i].dstBuf_, readOps_[i].size_ );
                hAtomic::LWMemoryBarrier();
                readOps_[i].done_ = hTrue;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hStreamingResourceBase::PollSteamRead( hUint32 opID, hUint32* read )
    {
        //hMutexAutoScope am( &lock_ );
        hcAssert( opID < MAX_READ_OPS );

        if ( readOps_[opID].active_ && readOps_[opID].done_ )
        {
            *read = readOps_[opID].read_;
            readOps_[opID].active_ = hFalse;
            return 0;
        }
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hStreamingResourceBase::~hStreamingResourceBase()
    {
        if ( fileStream_.IsOpen() )
        {
            fileStream_.Close();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceManager* hResourceHandle::manager_=hNullptr;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hResourceHandle::acquire() {
        hResourceClassBase* res=manager_->getResourceForHandle(resourceID_);
        if (res!=lastPtr_) {
            updateRequired_=hTrue;
        }
        if (res) {
            res->lock();
            acquired_=hTrue;
        }
        lastPtr_=res;
        return lastPtr_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hResourceHandle::weakPtr() const {
        return manager_->getResourceForHandle(resourceID_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceHandle::release() {
        if (lastPtr_) {
            lastPtr_->unlock();
            acquired_=hFalse;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceHandle::registerForUpdates(hResourceEventProc proc) {
        if (resourceID_!=0) {
            manager_->registerResourceEventHandler(resourceID_, proc);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceHandle::unregisterForUpdates(hResourceEventProc proc) {
        if (resourceID_!=0) {
            manager_->unregisterResourceEventHandler(resourceID_, proc);
        }
    }

}