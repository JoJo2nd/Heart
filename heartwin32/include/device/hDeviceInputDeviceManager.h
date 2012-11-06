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
    class hdSystemWindow;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	class HEARTDEV_SLIBEXPORT hdInputDeviceManager
	{
	public:

#define HEART_MAX_GAMEPADS (4)

		hdInputDeviceManager();
		virtual ~hdInputDeviceManager();

		hBool				            Initialise(hdSystemWindow* window);
		void				            Destory();
		void				            Update();
        void                            EndOfFrameUpdate();
        hdKeyboard*                     GetSystemKeyboard() const { return keyboard_; }
        hdMouse*                        GetSystemMouse() const { return mouse_; }
        hdGamepad*                      GetGamepad(hUint32 padIdx) { return padIdx < HEART_MAX_GAMEPADS ? pads_+padIdx : NULL; }

        static hUint32                  GetInputIDPairCount();
        static const hdInputIDPair*     GetInputIDPairArray();

	private:

        static const hdInputIDPair      s_deviceInputPairs[];

        hdSystemWindow* systemWindow_;
		hdKeyboard*		keyboard_;
        hdMouse*        mouse_;
        hdGamepad       pads_[HEART_MAX_GAMEPADS];
		//TODO add game pad
	};

}

#endif // DEVICEINPUTDEVICEMANAGER_H__