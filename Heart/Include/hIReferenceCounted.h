/********************************************************************
	created:	2010/05/15
	created:	15:5:2010   18:48
	filename: 	hIReferenceCounted.h	
	author:		James
	
	purpose:	
*********************************************************************/

#ifndef HCIREFERENCECOUNTED_H__
#define HCIREFERENCECOUNTED_H__

#include "hTypes.h"
#include "hDebugMacros.h"
#include "hAtomic.h"

namespace Heart
{
	class hIReferenceCounted 
	{
	public:
		hIReferenceCounted() 
			: reference_( (hUint32*)hAlignMalloc( sizeof(hUint32), 32 ) )
		{}
		virtual ~hIReferenceCounted() 
        {
            hDELETE reference_;
            reference_ = 0;
        }
		void			AddRef() const { hAtomic::Increment( reference_ ); }
		void			DecRef() const { hcAssert( reference_ > 0 ); hAtomic::Decrement( reference_ ); if ( *reference_ == 0 ) { OnZeroRef(); } }
		hUint32			GetRefCount() const { return *reference_; }
		
	protected:
		
		virtual void	OnZeroRef() const {};

		mutable hUint32*		reference_;
	};


	class hIAutoReferenceCounted 
	{
	public:
		hIAutoReferenceCounted() 
            : reference_( (hUint32*)hAlignMalloc( sizeof(hUint32), 32 ) )
		{
            *reference_ = 1;
        }
		void			AddRef() const { hAtomic::Increment( reference_ ); }
		void			DecRef() const { hAtomic::Decrement( reference_ ); if ( *reference_ == 0 ) { hDELETE this; } }

	protected:

		virtual ~hIAutoReferenceCounted() 
        {
            hDELETE reference_;
            reference_ = NULL;
        }

		mutable hUint32*			reference_;
	};
}

#endif // HCIREFERENCECOUNTED_H__