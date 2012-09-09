/********************************************************************

	filename: 	hClock.h	
	
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

#ifndef hcClock_h__
#define hcClock_h__

namespace Heart
{
	class hTimer;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	namespace hClock
	{
        HEARTBASE_SLIBEXPORT
		void			Initialise();
        HEARTBASE_SLIBEXPORT
		hFloat			elapsed();
        HEARTBASE_SLIBEXPORT
		hUint32			elapsedMS();
        HEARTBASE_SLIBEXPORT
		hUint32			deltams();
        HEARTBASE_SLIBEXPORT
		hFloat			fdeltams();
        HEARTBASE_SLIBEXPORT
		hFloat			Delta();
        HEARTBASE_SLIBEXPORT
		hUint32			hours();
        HEARTBASE_SLIBEXPORT
		hUint32			mins();
        HEARTBASE_SLIBEXPORT
		hUint32			secs();
        HEARTBASE_SLIBEXPORT
		void			Update();
        HEARTBASE_SLIBEXPORT
		void			BeginTimer( hTimer& timer );
        HEARTBASE_SLIBEXPORT
		void			EndTimer( hTimer& timer );
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class HEARTBASE_SLIBEXPORT hTimer
	{
	public:

		hUint32		ElapsedMS() {return deltaT_/1000;}
        hUint32		ElapsedmS() {return deltaT_;}

	private:

		friend void	hClock::BeginTimer(hTimer&);
		friend void	hClock::EndTimer(hTimer&);

		hUint64		beginT_;
		hUint32		deltaT_;
	};
}// namespace Heart

#endif // hcClock_h__