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


    class HEART_DLLEXPORT hResourceClassBase : public hMapElement< hUint32, hResourceClassBase >
    {
    public:
        hResourceClassBase() 
            : resourceID_( 0 )
            , linked_(hFalse)
        {}
        virtual ~hResourceClassBase() {}
        hUint32                 GetResourceID() const { return resourceID_; }
        void                    SetName(const hChar* name) { hStrCopy(name_.GetBuffer(), name_.GetMaxSize(), name); }
        const hChar*            GetName() const { return name_.GetBuffer(); }
        void                    SetType(const hResourceType& type) { type_ = type; }
        hResourceType           GetType() const { return type_; }
        void                    SetIsLinked(hBool val){ linked_ = val; }
        hBool                   GetIsLinked() const { return linked_; }

    protected:

        friend class hResourceManager;
        friend class hResoucePackageV2;

        hArray<hChar, 32>  name_;
        hUint32            resourceID_;
        hBool              linked_;
        hResourceType      type_;
    };

    class hStreamingResourceBase : public hResourceClassBase 
    {
    public:
        hStreamingResourceBase()
            : hResourceClassBase()
        {
            hZeroMem( &readOps_, sizeof(readOps_) );
            //flags_ |= ResourceFlags_STREAMING;
        }
        virtual ~hStreamingResourceBase();
        hUint QueueStreamRead( void* dstBuf, hUint32 size, hUint32 offset, hUint32* opID );
        hUint PollSteamRead( hUint32 opID, hUint32* read );

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