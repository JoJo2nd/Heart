/********************************************************************
	created:	2010/05/19
	created:	19:5:2010   21:29
	filename: 	Profiler.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef PROFILER_H__
#define PROFILER_H__

#include "hTypes.h"
#include "hClock.h"

namespace Heart
{
	class ProfilerManager
	{
	public:
		ProfilerManager();
		virtual ~ProfilerManager();

		void			BeginFrame();
		void			EndFrame();
		void			LogTime( const char* pFunction, hUint32 time );
	private:

	};

	extern ProfilerManager* gProfilerManager_;

	struct Profile
	{
		Profile( const char* pFunction, hUint32* pIdx ) :
			pFunction_( pFunction )
		{

			hClock::BeginTimer( timer_ );
		}

		~Profile()
		{
			hClock::EndTimer( timer_ );
			gProfilerManager_->LogTime( pFunction_, timer_.ElapsedmS() );
		}

		const char*				pFunction_;
		hTimer					timer_;
	};
}

#define PROFILE() static hUint32 __profileIdx__ = 0; Heart::Debug::Profile __xxx__( __FUNCTION__, &__profileIdx__ );

#endif // PROFILER_H__