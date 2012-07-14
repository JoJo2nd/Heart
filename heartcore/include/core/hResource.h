/********************************************************************

	filename: 	hResource.h	
	
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

#ifndef RESOURCE_H__
#define RESOURCE_H__


namespace Heart
{
    struct hResourceType
    {
        hChar						ext[4];

        hBool						operator == ( const hResourceType& b ) const 
        {
            return strcmp( ext, b.ext ) == 0;
        }
        hBool						operator != ( const hResourceType& b ) const 
        {
            return strcmp( ext, b.ext ) != 0;
        }
        hBool						operator < ( const hResourceType& b ) const
        {
            return strcmp( ext, b.ext ) < 0;
        }
    };


    class HEARTCORE_SLIBEXPORT hResourceClassBase : public hMapElement< hUint32, hResourceClassBase >,
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
        void    SetType(const hResourceType& type) { type_ = type; }
        hResourceType GetType() const { return type_; }

        friend class hResourceManager;

        hResourceManager*  manager_;
        hUint32            resourceID_;
        hUint32            flags_;
        hResourceType       type_;
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
        ResourceFlags QueueStreamRead( void* dstBuf, hUint32 size, hUint32 offset, hUint32* opID );
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
            hUint32 offset_;
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
#ifndef HEART_PACKER
    #define HEART_RESOURCE_SAFE_RELEASE( x ) { if (x) {x->DecRef(); x = NULL;} }
    #define HEART_RESOURCE_DATA_FIXUP( type, x, y ) y = (type*)((hByte*)x + (hUint32)y);
#else
    #define HEART_RESOURCE_SAFE_RELEASE( x ) x = NULL;
    #define HEART_RESOURCE_DATA_FIXUP( type, x, y ) /*y = (type*)((hByte*)x + (hUint32)y);*/
#endif

#endif // RESOURCE_H__