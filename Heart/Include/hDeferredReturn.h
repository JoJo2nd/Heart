/********************************************************************
	created:	2010/06/27
	created:	27:6:2010   20:48
	filename: 	DeferredReturn.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef DEFERREDRETURN_H__
#define DEFERREDRETURN_H__

#include "hTypes.h"
#include "hAtomic.h"

namespace Heart
{
	class hDeferredReturn
	{
	public:
		hDeferredReturn() : 
		  ready_( hFalse )
		{}
		void			SetReady() 
		{ 
			hAtomic::LWMemoryBarrier();
			ready_ = hTrue; 
		}
		hBool			IsReady() const { return ready_; }
	private:
		volatile hBool	ready_;
	};
}

#endif // DEFERREDRETURN_H__