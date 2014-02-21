/********************************************************************

	filename: 	hDeviceKeyboard.h	
	
	Copyright (c) 19:7:2012 James Moran
	
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

#ifndef DEVICEKEYBOARD_H__
#define DEVICEKEYBOARD_H__

namespace Heart
{
	class EventManager;

	class HEART_DLLEXPORT hdKeyboard
	{
	public:

		hdKeyboard();
		virtual ~hdKeyboard();

		hBool							IsAttacted() { return hTrue; }
		hUint32							AquireExclusiveAccess() const;
		void 							ReleaseExclusiveAccess( hUint32 accesskey ) const;
        hdInputButton                   GetButton( hdInputID buttonId ) const;
        hdInputButton                   GetButton( hdInputID buttonId, hUint32 accesskey ) const;
		const hChar*					GetCharBufferData() const { return charBuffer_; }
		hUint32							CharBufferSizeBytes() const { return nCharacters_; }
		void							FlushCharBufferData();
        void                            PushCharacterEvent(hChar ch);
        void                            SetButton(hdInputID buttonID, hdInputButtonState state);
		void							Update();
        void                            EndOfFrameUpdate();

	private:

		static const hUint32					MAX_BUFFER_SIZE = 64;
		static const hUint32					KEYBOARD_KEYS = 256;

		hdInputButton						    keys_[ KEYBOARD_KEYS ];
		hChar									charBuffer_[ MAX_BUFFER_SIZE + 1 ];// + 1 to account for the \0
		hUint32									nCharacters_;
		mutable hUint32							accessKey_;
	};

}

#endif // DEVICEKEYBOARD_H__