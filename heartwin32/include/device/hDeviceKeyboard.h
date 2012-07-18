/********************************************************************
	created:	2010/08/30
	created:	30:8:2010   18:30
	filename: 	DeviceKeyboard.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef DEVICEKEYBOARD_H__
#define DEVICEKEYBOARD_H__

namespace Heart
{
	class EventManager;

	class HEARTDEV_SLIBEXPORT hdKeyboard
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
		void							FlushCharBufferData() { nCharacters_ = 0; }
		void							Update();

	private:

		static const hUint32					MAX_BUFFER_SIZE = 64;
		static const hUint32					KEYBOARD_KEYS = 256;

		void									PushBufferEvent( hUint32 vKey, bool state );

		hdInputButton						    keys_[ KEYBOARD_KEYS ];
		hChar									charBuffer_[ MAX_BUFFER_SIZE + 1 ];// + 1 to account for the \0
		hUint32									nCharacters_;

		mutable hUint32							accessKey_;
	};

}

#endif // DEVICEKEYBOARD_H__