/********************************************************************
	created:	2010/08/30
	created:	30:8:2010   18:30
	filename: 	DeviceKeyboard.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef DEVICEKEYBOARD_H__
#define DEVICEKEYBOARD_H__

//#include "DeviceIInput.h"
//#include "hEventManager.h"
//#include "DeviceIKernel.h"

namespace Heart
{
	class EventManager;

namespace Device
{
	class Keyboard : public IBufferedInput, public ICharacterInput
	{
	public:

		Keyboard();
		virtual ~Keyboard();

		virtual void							Initialise( hUint32 port, EventManager* pEvManager );
		virtual void							Destroy();

		virtual hBool							IsAttacted() { return hTrue; }

		virtual hUint32							AquireExclusiveAccess() const;
		virtual void 							ReleaseExclusiveAccess( hUint32 accesskey ) const;

		virtual InputButton 					GetButton( Device::InputID buttonId ) const;
		virtual InputAxis						GetAxis( Device::InputID axisId ) const;

		virtual InputButton 					GetButton( InputID buttonId, hUint32 accesskey ) const;
		virtual InputAxis						GetAxis( InputID axisId, hUint32 accesskey ) const;

		virtual const InputBufferData*			GetBufferedData() const { return bufferedData_; }
		virtual hUint32							BufferSize() const { return nBufferData_; }
		virtual void							FlushBufferedData() { nBufferData_ = 0; charBuffer_[ 0 ] = '\0'; }

		virtual const hChar*					GetCharBufferData() const { return charBuffer_; }
		virtual hUint32							CharBufferSizeBytes() const { return nCharacters_; }
		virtual void							FlushCharBufferData() { nCharacters_ = 0; }

		virtual void							Update();


	private:

		typedef Listener< KernelEvents::KeyboardInputEvent >		KeyListener;
		typedef Listener< KernelEvents::KeyboardCharacterEvent >	CharacterListener;
		typedef Listener< KernelEvents::MouseMoveEvent >			MouseMoveListener;

		static const hUint32					MAX_BUFFER_SIZE = 64;
		static const hUint32					KEYBOARD_KEYS = 256;

		void									OnKeyPressEvent( hUint32 id, KernelEvents::KeyboardInputEvent& event );
		void									OnCharacterEvent( hUint32 id, KernelEvents::KeyboardCharacterEvent& event );
		void									OnMouseEvent( hUint32 id, KernelEvents::MouseMoveEvent& event );
		void									PushBufferEvent( hUint32 vKey, bool state );

		InputButton								keys_[ KEYBOARD_KEYS ];
		hUint32									nBufferData_;
		InputBufferData							bufferedData_[ MAX_BUFFER_SIZE ];
		
		hChar									charBuffer_[ MAX_BUFFER_SIZE + 1 ];// + 1 to account for the \0
		hUint32									nCharacters_;

		InputAxis								nextMouseX_;
		InputAxis								nextMouseY_;
		InputAxis								mouseX_;
		InputAxis								mouseY_;

		EventManager*							pEventManager_;
		KeyListener								keyListener_;
		CharacterListener						charListener_;
		MouseMoveListener						mouseListener_;

		mutable hUint32							accessKey_;
	};

}
}

#endif // DEVICEKEYBOARD_H__