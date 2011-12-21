/********************************************************************
	created:	2008/06/17
	created:	17:6:2008   20:34
	filename: 	hClock.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef hcClock_h__
#define hcClock_h__

#include "hTypes.h"

namespace Heart
{
	class hTimer;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	namespace hClock
	{
		void			Initialise();
		hFloat			elapsed();
		hUint32			elapsedMS();
		hUint32			deltams();
		hFloat			fdeltams();
		hFloat			Delta();
		hUint32			hours();
		hUint32			mins();
		hUint32			secs();
		void			Update();
		void			BeginTimer( hTimer& timer );
		void			EndTimer( hTimer& timer );
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hTimer
	{
	public:

		hUint32		ElapsedMS();
		hUint32		ElapsedmS();

	private:

		friend void	hClock::BeginTimer(hTimer&);
		friend void	hClock::EndTimer(hTimer&);

		hInt64		beginT_;
		hInt32		deltaT_;
	};
}// namespace Heart

#endif // hcClock_h__