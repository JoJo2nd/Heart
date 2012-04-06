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