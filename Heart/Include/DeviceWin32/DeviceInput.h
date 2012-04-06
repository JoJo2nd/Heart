/********************************************************************

	filename: 	DeviceInput.h	
	
	Copyright (c) 23:7:2011 James Moran
	
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
#ifndef DEVICEIINPUT_H__
#define DEVICEIINPUT_H__

#define HEART_INPUT_ANY_PORT (hErrorCode)

namespace Heart
{
	class EventManager;

namespace Device
{

	enum InputDeviceTypes
	{
		IDT_PAD,
		IDT_KEYBOARD,
		IDT_MOUSE,
	};

	enum InputID
	{
		//////////////////////////////////////////////////////////////////////////
		// Generic Button IDs ////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		InputID_FACEBUTTON1,
		InputID_FACEBUTTON2,
		InputID_FACEBUTTON3,
		InputID_FACEBUTTON4,
		InputID_FACEBUTTON5,
		InputID_FACEBUTTON6,
		InputID_FACEBUTTON7,
		InputID_FACEBUTTON8,

		InputID_DPADUP,
		InputID_DPADDOWN,
		InputID_DPADLEFT,
		InputID_DPADRIGHT,

		//////////////////////////////////////////////////////////////////////////
		// Mouse ID's/////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		IID_LEFTMOUSEBUTTON,
		IID_RIGHTMOUSEBUTTON,
		IID_MIDDLEMOUSEBUTTON,
		IID_MOUSEYAXIS,
		IID_MOUSEXAXIS,

		//////////////////////////////////////////////////////////////////////////
		// Keyboard ID's /////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		IID_KEYA ,
		IID_KEYB ,
		IID_KEYC ,
		IID_KEYD ,
		IID_KEYE ,
		IID_KEYF ,
		IID_KEYG ,
		IID_KEYH ,
		IID_KEYI ,
		IID_KEYJ ,
		IID_KEYK ,
		IID_KEYL ,
		IID_KEYM ,
		IID_KEYN ,
		IID_KEYO ,
		IID_KEYP ,
		IID_KEYQ ,
		IID_KEYR ,
		IID_KEYS ,
		IID_KEYT ,
		IID_KEYU ,
		IID_KEYV ,
		IID_KEYW ,
		IID_KEYX ,
		IID_KEYY ,
		IID_KEYZ ,
		IID_SPACE ,
		IID_1,
		IID_2,
		IID_3,
		IID_4,
		IID_5,
		IID_6,
		IID_7,
		IID_8,
		IID_9,
		IID_0,
		IID_CAPSLOCK,
		IID_LEFTCTRL,
		IID_RIGHTCTRL,
		IID_LEFTSHIFT,
		IID_RIGHTSHIFT,
		IID_TAB,
		IID_RETURN,
		IID_ALT,
		IID_UPARROW,
		IID_DOWNARROW,
		IID_LEFTARROW,
		IID_RIGHTARROW,
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	enum InputState
	{
		IS_UP,
		IS_DOWN = 1,
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	struct InputButton
	{
		InputButton() :
			buttonVal_( hFalse )
			,raisingEdge_( hFalse )
			,fallingEdge_( hFalse )
		{}
		hBool			buttonVal_		: 1;
		hBool			raisingEdge_	: 1;
		hBool			fallingEdge_	: 1;
		hBool			isRepeat_		: 1;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	struct InputAxis
	{
		InputAxis() : 
			anologueVal_( 0.0f ) 
		{}
		hFloat			anologueVal_;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class IInput
	{
	public:
		IInput() {}
		virtual ~IInput() {}

		virtual void		Initialise( hUint32 port, EventManager* pEvManager ) = 0;
		virtual void		Destroy() = 0;

		virtual hBool		IsAttacted() = 0;

		virtual hUint32		AquireExclusiveAccess() const = 0;
		virtual void 		ReleaseExclusiveAccess( hUint32 accesskey ) const = 0;

		virtual InputButton GetButton( InputID buttonId ) const = 0;
		virtual InputAxis	GetAxis( InputID axisId ) const = 0;

		virtual InputButton GetButton( InputID buttonId, hUint32 accesskey ) const = 0;
		virtual InputAxis	GetAxis( InputID axisId, hUint32 accesskey ) const = 0;

		virtual void		Update() = 0;

	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	struct InputBufferData
	{
		InputID			buttonID_;
		InputState		state_;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class IBufferedInput : public IInput
	{
	public:
		virtual ~IBufferedInput() {}

		virtual const InputBufferData*	GetBufferedData() const = 0;
		virtual hUint32					BufferSize() const = 0;
		virtual void					FlushBufferedData() = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
		
	class ICharacterInput
	{
	public: 
		virtual ~ICharacterInput() {}

		virtual const hChar*	GetCharBufferData() const = 0;
		virtual hUint32			CharBufferSizeBytes() const = 0;
		virtual void			FlushCharBufferData() = 0;
	};
}
}

#endif // DEVICEIINPUT_H__