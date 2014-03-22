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

#if 0 //hResourceType is replaced with Object Type ID
    struct hResourceType
    {
        hResourceType() {}
        explicit hResourceType(const hChar* name) 
            : typeCRC(hCRC32::StringCRC(name))
        {}
        explicit hResourceType(hUint32 cc)
            : typeCRC(cc)
        {}
        hUint32 typeCRC;

        hBool operator == ( const hResourceType& b ) const {
            return typeCRC == b.typeCRC;
        }
        hBool operator != ( const hResourceType& b ) const {
            return typeCRC != b.typeCRC;
        }
        hBool operator < ( const hResourceType& b ) const {
            return typeCRC < b.typeCRC;
        }
    };
#endif

#if 0 // ResourceID is replaced with hStringID
    struct hResourceID
    {
        hUint64 hash_;

        hResourceID() : hash_(0) {}
        explicit hResourceID(hUint64 v) : hash_(v) {}
        hResourceID(const hResourceID& rhs) : hash_(rhs.hash_) {}
        hResourceID(hUint32 pkgid, hUint32 pkgresid) : hash_((hUint64)(((hUint64)pkgid << 32) | ((hUint64)pkgresid))) {}
        hBool operator == (const hResourceID& rhs) const { return hash_ == rhs.hash_; }
        hBool operator != (const hResourceID& rhs) const { return hash_ != rhs.hash_; }
        hResourceID& operator = (const hResourceID& rhs) { hash_ = rhs.hash_; return *this; }
        operator hUint64() { return hash_; }

        struct hash
        {
            std::size_t operator()(hResourceID const& val) const {
                return val.hash_;
            }
        };

        //TODO: Remove this!!
        static hResourceID buildResourceID(const hChar* resourceName){
            return buildResourceID(hStringID(resourceName));
        }
        static hResourceID buildResourceID(const hStringID& fullPath){
            if (fullPath.is_default())
                return hResourceID();

            return hResourceID((hUint64)fullPath.hash());
//             const hChar* resName = hStrChr(fullPath, '.');
//             if (!resName)
//                 return hResourceID();
//             hUint32 pakCRC = hCRC32::FullCRC(fullPath, (hUint32)resName-(hUint32)fullPath);
//             hUint32 resCRC = hCRC32::StringCRC(resName+1);
// 
//             return hResourceID((hUint64)(((hUint64)pakCRC << 32) | ((hUint64)resCRC)));
        }
        static hResourceID buildResourceID(const hChar* package, const hChar* resourceName){
            if (!package || !resourceName)
                return hResourceID();

            hUint32 pakCRC = hCRC32::StringCRC(package);
            hUint32 resCRC = hCRC32::StringCRC(resourceName);

            return  hResourceID((hUint64)(((hUint64)pakCRC << 32) | ((hUint64)resCRC)));
        }
    };
#endif

    typedef std::vector< hStringID > hResourceNodeLinks;

    namespace hResourceColour
    {
        enum Type {
            White,
            Grey,
            Black,
        };
    }

    struct hResourceGraphNode
    {
        hResourceGraphNode()
            : resourceData_(nullptr)
            , colour_(hResourceColour::White)
        {

        }

        void*                   resourceData_;
        hStringID               typeID_;
        hResourceNodeLinks      links_;
        hResourceColour::Type   colour_;
    };

    enum hResurceEvent 
    {
        hResourceEvent_None,
        hResourceEvent_Created,
        hResourceEvent_Linked,
        hResourceEvent_Unlinked,
        hResourceEvent_Unloaded,

        hResourceEvent_DBInsert,
        hResourceEvent_DBRemove,
        hResourceEvent_HotSwap,
    };

#if 0 // Removing, we don't want to constrain resources to fix our interface
    class HEART_DLLEXPORT hResourceClassBase : public hMapElement< hUint32, hResourceClassBase >
    {
    public:
        hResourceClassBase() 
            : resourceID_( 0 )
        {}
        virtual ~hResourceClassBase() {}
        void                    setResourceID(const hResourceID& resid) { resourceID_=resid; }
        hResourceID             getResourceID() const { return resourceID_; }
        void                    SetName(const hChar* name) { hStrCopy(name_.GetBuffer(), name_.GetMaxSize(), name); }
        const hChar*            GetName() const { return name_.GetBuffer(); }
        void                    SetType(const hResourceType& type) { type_ = type; }
        hResourceType           GetType() const { return type_; }
//         hBool                   getIsLocked() const { return lockedCount_ > 0; }
//         hUint                   getLockCount() const { return lockedCount_; }
//         void                    lock() { ++lockedCount_; }
//         void                    unlock() { hcAssert(lockedCount_ > 0); --lockedCount_; }

    protected:

        friend class hResourceManager;
        friend class hResoucePackageV2;

        hArray<hChar, 32>  name_;
        hResourceID        resourceID_;
        hResourceType      type_;
    };
#endif

    class hResourceManager;

#if 0 // hResourceEventProc is replaced by hNewResoruceEventProc (awaiting rename)
    hFUNCTOR_TYPEDEF(hBool (*)(hResourceID , hResurceEvent, hResourceManager*, hResourceClassBase*), hResourceEventProc);
#endif
    hFUNCTOR_TYPEDEF(hBool (*)(hStringID/*res_id*/, hResurceEvent/*event_type*/, hStringID/*type_id*/, void*/*data_ptr*/), hNewResourceEventProc);

    class hResourceHandle
    {
    public:

        hResourceHandle()
            : resourceID_(0)
            , flags_(0)
        {

        }
        explicit hResourceHandle(hStringID resid)
            : resourceID_(resid)
            , flags_(0)
        {

        }
        explicit hResourceHandle(const hChar* path)
            : resourceID_()
            , flags_(0)
        {

        }
        ~hResourceHandle()
        {
        }
        hResourceHandle(const hResourceHandle& rhs) 
            : resourceID_(rhs.resourceID_)
            , flags_(rhs.flags_)
        {
            registeredForUpdates_=hFalse;
        }
        /*hResourceHandle(hResourceHandle&& rhs) {
            swap(this, &rhs);
        }
        hResourceHandle& operator = (hResourceHandle&& rhs) {
            swap(this, &rhs);
            return *this;
        }*/
        hResourceHandle& operator = (hResourceHandle rhs) {
            swap(this, &rhs);
            return *this;
        }

        template< typename t_ty >
        t_ty*               weakPtr() const {
            return manager_->getResourceForHandle<t_ty>(resourceID_);
        }
        void                registerForUpdates(hNewResourceEventProc proc);
        void                unregisterForUpdates(hNewResourceEventProc proc);
        hBool               getIsValid() const { return !resourceID_.is_default(); }
        hStringID           getResourceID() const { return resourceID_; }

    private:

        friend class hResourceManager;

        hResourceHandle(hResourceManager* manager, hStringID res_id)
            : resourceID_(res_id)
            , flags_(0)
        {
            if (!manager_) {
                manager_=manager;
            }
        }

        static void swap(hResourceHandle* lhs, hResourceHandle* rhs) {
            std::swap(lhs->manager_, rhs->manager_);
            std::swap(lhs->resourceID_, rhs->resourceID_);
            std::swap(lhs->flags_, rhs->flags_);
        }

        static hResourceManager*    manager_;
        hStringID                   resourceID_;
        union {
            hUint                   flags_;
            struct {
                hBool               registeredForUpdates_; //this flag should not be copied?
            };
        };
    };

    template< typename t_ty >
    class hResourceHandleScope
    {
    public:
        hResourceHandleScope(hResourceHandle& handle, t_ty** ptr)
            : addr_(ptr)
            , handle_(handle)
        {
            *addr_ = handle_.weakPtr<t_ty>();
        }

        ~hResourceHandleScope()
        {
            *addr_=hNullptr;
        }

        operator hBool () const {
            return *addr_ != hNullptr;
        }

        hResourceHandle& handle_;
        t_ty**           addr_;
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