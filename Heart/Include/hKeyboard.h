/********************************************************************

	filename: 	hKeyboard.h	
	
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

#ifndef KEYBOARD_H__
#define KEYBOARD_H__

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

		hBool							Initialise( Device::ICharacterInput* pKbImpl, Device::IInput* pButtonKeyboard, Device::IBufferedInput* bufferKeyb );
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
        hUint32                         GetBufferKeysSize() const { return bufferInput_->BufferSize(); }
        const Device::InputBufferData*  GetBufferedKeys() const { return bufferInput_->GetBufferedData(); }

	private:

		friend class hControllerManager;

		Device::IInput*					pButtonKeyboard_;
		Device::ICharacterInput*		pKeyboard_;
        Device::IBufferedInput*         bufferInput_;

	};
}

#endif // KEYBOARD_H__