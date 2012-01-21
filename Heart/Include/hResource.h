/********************************************************************
	created:	2010/09/11
	created:	11:9:2010   23:12
	filename: 	hResource.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef RESOURCE_H__
#define RESOURCE_H__

#include "hTypes.h"
#include "hIReferenceCounted.h"
#include "hDeferredReturn.h"
#include "hLinkedList.h"
#include "hMap.h"

namespace Heart
{
	class hResourceClassBase : public hMapElement< hUint32, hResourceClassBase >,
							   public hIReferenceCounted
	{
	private:
		enum ResourceFlags
		{
			ResourceFlags_DISKRESOURCE	= 1,
			ResourceFlags_FULLYLOADED	= 1 << 1,
		};

	public:
		hResourceClassBase() 
			: flags_( 0 )
			, manager_( NULL )
			, resourceID_( 0 )
		{}
		hBool					IsDiskResource() const { return (flags_ & ResourceFlags_DISKRESOURCE) == ResourceFlags_DISKRESOURCE; }
		void					IsDiskResource( hBool val ) { flags_ = val ? (flags_ | ResourceFlags_DISKRESOURCE) : (flags_ & ~ResourceFlags_DISKRESOURCE); }
        hUint32 				GetResourceID() const { return resourceID_; }

	private:

		void	OnZeroRef() const;
		void	SetResID( hUint32 id ) { resourceID_ = id; }

		friend class hResourceManager;
		friend class hResourceDependencyList;

		hResourceManager*				manager_;
		hUint32							resourceID_;
		hUint32							flags_;
	};

#define HEART_RESOURCE_SAFE_RELEASE( x ) { if (x) {x->DecRef(); x = NULL;} }

	/*
		JM TODO: Remove these templated classes. They were an interesting idea
		but in the end its simpler and cleaner to have all resources derive from a 
		base class (i.e. hResourceClassBase). The idea was to allow the resource manager to
		not worry about the types of the classes it handles but using a template and handle method
		just makes it difficult to cast and work with. On top of this it adds an extra indirection
		to accessing resources. As resources are pretty core to Heart, it makes sense that they
		are easy to use and (hopefully) quick to access.
		Also, as a plus, having a base class allows us to force some common traits on to resources.
		I ended up doing this any way with ResourceBaseClass so may as well make it proper. As they
		say K.I.S.S.

		Sadly, its a big change so I'll work it in at a later date...
	*/

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	template< typename _Ty >
	class hResource : public hIReferenceCounted
	{
	public:

		_Ty* pData() const { return pData_; }
		hBool IsLoaded() const { return dataValid_.IsReady(); }

	private:

		friend class hResourceManager;
		friend struct hLoadedResourceInfo;

		hResource() {};
		void					OnZeroRef() const;

		ClassType< _Ty >		typeInfo_;	
		hResourceManager*		pManager_;
		_Ty*					pData_;
		hDeferredReturn			dataValid_;

	};
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	template< typename _Ty >
	class hResourceHandle
	{
	public:
		hResourceHandle() :
		  pResData_( NULL )
		  {}

		_Ty* operator -> () { return pResData_->pData(); }
		const _Ty* operator -> () const { return pResData_->pData(); }
		operator _Ty*() const { return pResData_->pData(); }
		//operator const _Ty*() const { return pResData_->pData(); }
		bool operator == ( const hResourceHandle& b )
		{
			return pResData_ == b.pResData_;
		}

		template< typename _Sy >
		operator hResourceHandle< _Sy >& ()
		{
			//Get the compiler to double check that the 
			//cast being done is not totally stupid
			_Ty* t = NULL;
			_Sy* s = t;
			return *(reinterpret_cast< hResourceHandle< _Sy >* >(this));
		}

		void Acquire() const // acquire 
		{
			hcAssertMsg( pResData_, "Error Aquiring a NULL resource" );	
			pResData_->AddRef();
		}

		void Release() const
		{
		  hcAssertMsg( pResData_, "Error Releasing a NULL resource" );
		  pResData_->DecRef();
		  pResData_ = NULL;
		}

		hBool IsLoaded() const { return pResData_ && pResData_->IsLoaded(); }
		bool HasData() const { hcAssert( pResData_ != (hResource< _Ty >*)0xCDCDCDCD ); return pResData_ != NULL; }
		void SetResID( hUint32 id ) { resourceID_ = id; if ( id == 0 ) pResData_ = NULL; }
		hUint32 GetResID() const { return resourceID_; }

	public:

		friend class hResourceManager;
		friend class hResourceDependencyList;

		mutable hResource< _Ty >*			pResData_;
		hUint32								resourceID_;
	};
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#define HEART_RESOURCE_DATA_FIXUP( type, x, y ) y = (type*)((hByte*)x + (hUint32)y);

#endif // RESOURCE_H__