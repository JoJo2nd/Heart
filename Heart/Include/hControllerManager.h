/********************************************************************
	created:	2009/11/28
	created:	28:11:2009   23:38
	filename: 	hControllerManager.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HOCONTROLLERMANAGER_H__
#define HOCONTROLLERMANAGER_H__

#include "hTypes.h"
#include "hController.h"
#include "hKeyboard.h"

namespace Heart
{
namespace Events
{
	class EventManager;
}

	class hControllerManager : public pimpl< Device::InputDeviceManager >
	{
	public:

		hControllerManager();
		~hControllerManager();

		hBool						Initialise( EventManager* pEvManager ) { return pImpl()->Initialise( pEvManager ); }
		void						InitPad( hUint32 port, const ControllerConfig* config )
		{
			controllers_[ port ].SetPadPort( port );
		}
		hController*					pController( hUint32 i )
		{
			return &controllers_[ i ];
		}
		const hKeyboard*				GetSystemKeyboard() const { return &keyboard_; }
		void						Update()
		{
			pImpl()->Update();

			if ( !keyboard_.pKeyboard_ )
			{
				Device::IInput* pKb = pImpl()->GetInputDevice( Device::IDT_KEYBOARD, HEART_INPUT_ANY_PORT );
				Device::ICharacterInput* pCKb = pImpl()->GetCharacterInputDevice( Device::IDT_KEYBOARD, HEART_INPUT_ANY_PORT );
				keyboard_.Initialise( pCKb, pKb );
			}

			for ( hUint32 i = 0; i < 4; ++i )
			{
				controllers_[ i ].Update();
			}
		}

	private:
		
		hUint32						nControllers_;
		hController					controllers_[ 4 ];
		hKeyboard					keyboard_;

	};
}

#endif // HOCONTROLLERMANAGER_H__