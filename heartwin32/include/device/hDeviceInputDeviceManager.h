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
		hdInputDeviceManager();
		virtual ~hdInputDeviceManager();

		hBool				            Initialise(hdSystemWindow* window);
		void				            Destory();
		void				            Update();
        void                            EndOfFrameUpdate();
        hdKeyboard*                     GetSystemKeyboard() const { return keyboard_; }

        static hUint32                  GetInputIDPairCount();
        static const hdInputIDPair*     GetInputIDPairArray();

	private:

        static const hdInputIDPair      s_deviceInputPairs[];

        hdSystemWindow* systemWindow_;
		hdKeyboard*		keyboard_;
        hdMouse*        mouse_;
		//TODO add game pad
	};

}

#endif // DEVICEINPUTDEVICEMANAGER_H__