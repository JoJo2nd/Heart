/********************************************************************
	created:	2010/06/27
	created:	27:6:2010   23:15
	filename: 	DeviceEvent.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef DEVICEEVENT_H__
#define DEVICEEVENT_H__

//#include "hTypes.h"
//#include <windows.h>

namespace Heart
{
namespace Device
{
	class ThreadEvent
	{
	public:
		ThreadEvent() :
			Event_( NULL )
		{
			Event_ = CreateEvent( NULL, FALSE, FALSE, NULL );
			hcAssert( Event_ != NULL );
		}

		virtual ~ThreadEvent()
		{
			hcAssert( Event_ != NULL );
			CloseHandle( Event_ );
		}

		void WaitOnSignal()
		{
			hcAssert( Event_ != NULL );
			WaitForSingleObject( Event_, INFINITE );
		}

		hBool TryOnSignal()
		{
			hcAssert( Event_ != NULL );
			DWORD ret = WaitForSingleObject( Event_, 0 );
			return ret == WAIT_OBJECT_0 ? hTrue : hFalse;
		}

		void Signal()
		{
			hcAssert( Event_ != NULL );
			SetEvent( Event_ );
		}

	private:

		HANDLE		Event_;
	};
}
}

#endif // DEVICEEVENT_H__