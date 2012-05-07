/********************************************************************
	created:	2010/09/04
	created:	4:9:2010   11:38
	filename: 	DeviceInputDeviceManager.h	
	author:		James
	
	purpose:	
*********************************************************************/

#ifndef DEVICEINPUTDEVICEMANAGER_H__
#define DEVICEINPUTDEVICEMANAGER_H__

// #include "DeviceIInputDeviceManager.h"
// #include "DeviceKeyboard.h"
// #include "DeviceMouse.h"

namespace Heart
{
namespace Device
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	class InputDeviceManager// : public IInputDeviceManager
	{
	public:
		InputDeviceManager();
		virtual ~InputDeviceManager();

		hBool				Initialise( EventManager* pEvManager );
		void				Destory();
		void				Update();
		IInput*				GetInputDevice( InputDeviceTypes type, hUint32 port );
		ICharacterInput*	GetCharacterInputDevice( InputDeviceTypes type, hUint32 port );
        IBufferedInput*     GetBufferInputDevice( InputDeviceTypes type, hUint32 port );

	private:

		Keyboard		keyboard_;
		//Mouse			mouse_;
		//TODO add game pad
	};
}
}

#endif // DEVICEINPUTDEVICEMANAGER_H__