/********************************************************************
	created:	2010/09/04
	created:	4:9:2010   18:53
	filename: 	Keyboard.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef KEYBOARD_H__
#define KEYBOARD_H__

#include "hTypes.h"

namespace Heart
{
namespace Device
{
	class ICharacterInput;
}
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	class hKeyboard
	{
	public:
		hKeyboard();
		~hKeyboard();

		hBool							Initialise( Device::ICharacterInput* pKbImpl, Device::IInput* pButtonKeyboard );
		hUint32							AquireExclusiveAccess() const { return pButtonKeyboard_->AquireExclusiveAccess(); }
		void							ReleaseExclusiveAccess( hUint32 accessKey ) const { pButtonKeyboard_->ReleaseExclusiveAccess( accessKey ); }
		hBool							GetButtonDown( Device::InputID id ) const;
		hBool							GetButtonPressed( Device::InputID id ) const;
		hBool							GetButtonUp( Device::InputID id ) const;
		hBool							GetButtonDown( Device::InputID id, hUint32 accessKey ) const;
		hBool							GetButtonPressed( Device::InputID id, hUint32 accessKey ) const;
		hBool							GetButtonUp( Device::InputID id, hUint32 accessKey ) const;
		Device::InputAxis				GetAxis( Device::InputID id ) const;
		const hChar*					GetBufferedText() const;

	private:

		friend class hControllerManager;

		Device::IInput*					pButtonKeyboard_;
		Device::ICharacterInput*		pKeyboard_;
	};
}

#endif // KEYBOARD_H__