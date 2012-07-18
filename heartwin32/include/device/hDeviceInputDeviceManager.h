/********************************************************************
	created:	2010/09/04
	created:	4:9:2010   11:38
	filename: 	DeviceInputDeviceManager.h	
	author:		James
	
	purpose:	
*********************************************************************/

#ifndef DEVICEINPUTDEVICEMANAGER_H__
#define DEVICEINPUTDEVICEMANAGER_H__

namespace Heart
{
namespace Device
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	class HEARTDEV_SLIBEXPORT hdInputDeviceManager// : public IInputDeviceManager
	{
	public:
		hdInputDeviceManager();
		virtual ~hdInputDeviceManager();

		hBool				Initialise();
		void				Destory();
		void				Update();

	private:

		hdKeyboard		keyboard_;
		//Mouse			mouse_;
		//TODO add game pad
	};
}
}

#endif // DEVICEINPUTDEVICEMANAGER_H__