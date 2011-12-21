/********************************************************************
	created:	2008/06/08
	created:	8:6:2008   15:37
	filename: 	hThreadEvent.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef hcEvent_h__
#define hcEvent_h__

#include "hTypes.h"
#include "DeviceEvent.h"

namespace Heart
{

	class hThreadEvent : public Device::ThreadEvent
	{
	public:
		hThreadEvent() {}
		virtual ~hThreadEvent() {}

		void Wait()
		{
			Device::ThreadEvent::WaitOnSignal();
		}

		hBool TryWait()
		{
			return Device::ThreadEvent::TryOnSignal();
		}

		void Signal()
		{
			Device::ThreadEvent::Signal();
		}

	private:
	};

}

#endif // hcEvent_h__