/********************************************************************

	filename: 	hControllerManager.h	
	
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

#ifndef HOCONTROLLERMANAGER_H__
#define HOCONTROLLERMANAGER_H__

namespace Heart
{
namespace Events
{
	class EventManager;
}

	class hControllerManager : public hPtrImpl< Device::InputDeviceManager >
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
                Device::IBufferedInput* pBKb = pImpl()->GetBufferInputDevice( Device::IDT_KEYBOARD, HEART_INPUT_ANY_PORT );
				keyboard_.Initialise( pCKb, pKb, pBKb );
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