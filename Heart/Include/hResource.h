/********************************************************************
    created:    2010/09/11
    created:    11:9:2010   23:12
    filename:     hResource.h    
    author:        James
    
    purpose:    
*********************************************************************/
#ifndef RESOURCE_H__
#define RESOURCE_H__

#include "hTypes.h"
#include "hIReferenceCounted.h"
#include "hDeferredReturn.h"
#include "hLinkedList.h"
#include "hMap.h"
#include "hSerialiserFileStream.h"

namespace Heart
{

    class hResourceClassBase : public hMapElement< hUint32, hResourceClassBase >,
                               public hIReferenceCounted
    {
    protected:
        enum ResourceFlags
        {
            ResourceFlags_OK            = 0,
            ResourceFlags_DISKRESOURCE  = 1,
            ResourceFlags_FULLYLOADED   = 1 << 1,
            ResourceFlags_BUSY          = 1 << 2,
            ResourceFlags_INVALID       = 1 << 3,
            ResourceFlags_STREAMING     = 1 << 4,
        };

    public:
        hResourceClassBase() 
            : flags_( 0 )
            , manager_( NULL )
            , resourceID_( 0 )
        {}
        virtual ~hResourceClassBase() {}
        hBool                   IsDiskResource() const { return (flags_ & ResourceFlags_DISKRESOURCE) == ResourceFlags_DISKRESOURCE; }
        void                    IsDiskResource( hBool val ) { flags_ = val ? (flags_ | ResourceFlags_DISKRESOURCE) : (flags_ & ~ResourceFlags_DISKRESOURCE); }
        hUint32                 GetResourceID() const { return resourceID_; }
        hUint32                 GetFlags() const { return flags_; }

    protected:

        void    OnZeroRef() const;
        void    SetResID( hUint32 id ) { resourceID_ = id; }

        friend class hResourceManager;

        hResourceManager*  manager_;
        hUint32            resourceID_;
        hUint32            flags_;
    };

    class hStreamingResourceBase : public hResourceClassBase 
    {
    public:
        hStreamingResourceBase()
            : hResourceClassBase()
        {
            hZeroMem( &readOps_, sizeof(readOps_) );
            flags_ |= ResourceFlags_STREAMING;
        }
        virtual ~hStreamingResourceBase();
        ResourceFlags QueueStreamRead( void* dstBuf, hUint32 size, hUint32* opID );
        ResourceFlags PollSteamRead( hUint32 opID, hUint32* read );

    private:

        friend class hResourceManager;

        struct ReadOp
        {
            hBool   active_;
            hBool   done_;
            hUint32 read_;
            void*   dstBuf_;
            hUint32 size_;
        };

        static const hUint32 MAX_READ_OPS = 8;

        void                            SetFileStream( const hSerialiserFileStream& stream ) { fileStream_ = stream; }
        void                            UpdateFileOps();

        hArray< ReadOp, MAX_READ_OPS >  readOps_;
        hSerialiserFileStream           fileStream_;
        //hMutex                          lock_;
    };

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#define HEART_RESOURCE_SAFE_RELEASE( x ) { if (x) {x->DecRef(); x = NULL;} }
#define HEART_RESOURCE_DATA_FIXUP( type, x, y ) y = (type*)((hByte*)x + (hUint32)y);

#endif // RESOURCE_H__