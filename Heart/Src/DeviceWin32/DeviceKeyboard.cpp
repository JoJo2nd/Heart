/********************************************************************

	filename: 	DeviceKeyboard.cpp	
	
	Copyright (c) 31:3:2012 James Moran
	
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


namespace Heart
{
namespace Device
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Keyboard::Keyboard() :
		pEventManager_( NULL )
		,nBufferData_( 0 )
		,accessKey_( 0 )
	{
		memset( keys_, 0, sizeof( keys_ ) );
		keyListener_.SetDelegate( KeyListener::Delegate::bind< Keyboard, &Keyboard::OnKeyPressEvent >( this ) );
		charListener_.SetDelegate( CharacterListener::Delegate::bind< Keyboard, &Keyboard::OnCharacterEvent >( this ) );
		mouseListener_.SetDelegate( MouseMoveListener::Delegate::bind< Keyboard, &Keyboard::OnMouseEvent >( this ) );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Keyboard::~Keyboard()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Keyboard::Initialise( hUint32 port, EventManager* pEvManager )
	{
		hcAssertMsg( pEvManager, "Event Manager is NULL" );

		pEventManager_ = pEvManager;

		pEventManager_->AddListener( KERNEL_EVENT_CHANNEL, &keyListener_ );
		pEventManager_->AddListener( KERNEL_EVENT_CHANNEL, &charListener_ );
		pEventManager_->AddListener( KERNEL_EVENT_CHANNEL, &mouseListener_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Keyboard::Destroy()
	{
		pEventManager_->RemoveListener( KERNEL_EVENT_CHANNEL, &keyListener_ );
		pEventManager_->RemoveListener( KERNEL_EVENT_CHANNEL, &charListener_ );
		pEventManager_->RemoveListener( KERNEL_EVENT_CHANNEL, &mouseListener_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Device::InputButton Keyboard::GetButton( Device::InputID buttonId ) const
	{
		return GetButton( buttonId, 0 );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Heart::Device::InputButton Keyboard::GetButton( InputID buttonId, hUint32 accesskey ) const
	{
		//someone else has exclusive access to the keyboard?
		if ( accessKey_ != accesskey )
		{
			return Device::InputButton();
		}

		switch ( buttonId )
		{
		case IID_KEYA:
			return keys_[ 'A' ];
		case IID_KEYB:
			return keys_[ 'B' ];
		case IID_KEYC:
			return keys_[ 'C' ];
		case IID_KEYD:
			return keys_[ 'D' ];
		case IID_KEYE:
			return keys_[ 'E' ];
		case IID_KEYF:
			return keys_[ 'F' ];
		case IID_KEYG:
			return keys_[ 'G' ];
		case IID_KEYH:
			return keys_[ 'H' ];
		case IID_KEYI:
			return keys_[ 'I' ];
		case IID_KEYJ:
			return keys_[ 'J' ];
		case IID_KEYK:
			return keys_[ 'K' ];
		case IID_KEYL:
			return keys_[ 'L' ];
		case IID_KEYM:
			return keys_[ 'M' ];
		case IID_KEYN:
			return keys_[ 'N' ];
		case IID_KEYO:
			return keys_[ 'O' ];
		case IID_KEYP:
			return keys_[ 'P' ];
		case IID_KEYQ:
			return keys_[ 'Q' ];
		case IID_KEYR:
			return keys_[ 'R' ];
		case IID_KEYS:
			return keys_[ 'S' ];
		case IID_KEYT:
			return keys_[ 'T' ];
		case IID_KEYU:
			return keys_[ 'U' ];
		case IID_KEYV:
			return keys_[ 'V' ];
		case IID_KEYW:
			return keys_[ 'W' ];
		case IID_KEYX:
			return keys_[ 'X' ];
		case IID_KEYY:
			return keys_[ 'Y' ];
		case IID_KEYZ:
			return keys_[ 'Z' ];
		case IID_SPACE:
			return keys_[ VK_SPACE ];
		case IID_1:
			return keys_[ '1' ];
		case IID_2:
			return keys_[ '2' ];
		case IID_3:
			return keys_[ '3' ];
		case IID_4:
			return keys_[ '4' ];
		case IID_5:
			return keys_[ '5' ];
		case IID_6:
			return keys_[ '6' ];
		case IID_7:
			return keys_[ '7' ];
		case IID_8:
			return keys_[ '8' ];
		case IID_9:
			return keys_[ '9' ];
		case IID_0:
			return keys_[ '0' ];
		case IID_CAPSLOCK:
			return keys_[ VK_CAPITAL ];
		case IID_LEFTCTRL:
			return keys_[ VK_LCONTROL ];
		case IID_RIGHTCTRL:
			return keys_[ VK_RCONTROL ];
		case IID_LEFTSHIFT:
			return keys_[ VK_LSHIFT ];
		case IID_RIGHTSHIFT:
			return keys_[ VK_RSHIFT ];
		case IID_TAB:
			return keys_[ VK_TAB ];
		case IID_RETURN:
			return keys_[ VK_RETURN ];
		case IID_ALT:
			return keys_[ VK_MENU ];
		case IID_UPARROW:
			return keys_[ VK_UP ];
		case IID_DOWNARROW:
			return keys_[ VK_DOWN ];
		case IID_LEFTARROW:
			return keys_[ VK_LEFT ];
		case IID_RIGHTARROW:
			return keys_[ VK_RIGHT ];
		}

		return Device::InputButton();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Device::InputAxis Keyboard::GetAxis( Device::InputID axisId ) const
	{
		return GetAxis( axisId, 0 );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Heart::Device::InputAxis Keyboard::GetAxis( InputID axisId, hUint32 accesskey ) const
	{
		//someone else has exclusive access to the keyboard?
		if ( accessKey_ != accesskey )
		{
			return Device::InputAxis();
		}

		switch( axisId )
		{
		case IID_MOUSEXAXIS:
			return mouseX_;
		case IID_MOUSEYAXIS:
			return mouseY_;
		}

		return Device::InputAxis();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Keyboard::Update()
	{
		FlushBufferedData();
		FlushCharBufferData();

		mouseX_.anologueVal_ = nextMouseX_.anologueVal_;
		mouseY_.anologueVal_ = nextMouseY_.anologueVal_;

		nextMouseX_.anologueVal_ = 0.0f;
		nextMouseY_.anologueVal_ = 0.0f;

		for ( hUint32 i = 0; i < KEYBOARD_KEYS; ++i )
		{
			if ( keys_[ i ].raisingEdge_ )
			{
				keys_[ i ].buttonVal_ = hTrue;
				PushBufferEvent( i, true );
			}
			if ( keys_[ i ].raisingEdge_ )
			{
				keys_[ i ].raisingEdge_ = hFalse;
			}

			if ( keys_[ i ].fallingEdge_ )
			{
				keys_[ i ].buttonVal_ = hFalse;
				PushBufferEvent( i, false );
			}
			if ( keys_[ i ].fallingEdge_ )
			{
				keys_[ i ].fallingEdge_ = hFalse;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Keyboard::PushBufferEvent( hUint32 vKey, bool state )
	{
		hcWarningHigh( nBufferData_ == MAX_BUFFER_SIZE, "Keyboard buffer overflow" );
		InputID id;

		switch( vKey )
		{
		case 'A': id = IID_KEYA; break;
		case 'B': id = IID_KEYB; break;
		case 'C': id = IID_KEYC; break;
		case 'D': id = IID_KEYD; break;
		case 'E': id = IID_KEYE; break;
		case 'F': id = IID_KEYF; break;
		case 'G': id = IID_KEYG; break;
		case 'H': id = IID_KEYH; break;
		case 'I': id = IID_KEYI; break;
		case 'J': id = IID_KEYJ; break;
		case 'K': id = IID_KEYK; break;
		case 'L': id = IID_KEYL; break;
		case 'M': id = IID_KEYM; break;
		case 'N': id = IID_KEYN; break;
		case 'O': id = IID_KEYO; break;
		case 'P': id = IID_KEYP; break;
		case 'Q': id = IID_KEYQ; break;
		case 'R': id = IID_KEYR; break;
		case 'S': id = IID_KEYS; break;
		case 'T': id = IID_KEYT; break;
		case 'U': id = IID_KEYU; break;
		case 'V': id = IID_KEYV; break;
		case 'W': id = IID_KEYW; break;
		case 'X': id = IID_KEYX; break;
		case 'Y': id = IID_KEYY; break;
		case 'Z': id = IID_KEYZ; break;
		case VK_SPACE: id = IID_SPACE; break;
		case '1': id = IID_1; break;
		case '2': id = IID_2; break;
		case '3': id = IID_3; break;
		case '4': id = IID_4; break;
		case '5': id = IID_5; break;
		case '6': id = IID_6; break;
		case '7': id = IID_7; break;
		case '8': id = IID_8; break;
		case '9': id = IID_9; break;
		case '0': id = IID_0; break;
		case VK_CAPITAL: id = IID_CAPSLOCK; break;
		case VK_LCONTROL: id = IID_LEFTCTRL; break;
		case VK_RCONTROL: id = IID_RIGHTCTRL; break;
		case VK_LSHIFT: id = IID_LEFTSHIFT; break;
		case VK_RSHIFT: id = IID_RIGHTSHIFT; break;
		case VK_TAB: id = IID_TAB; break;
		case VK_RETURN: id = IID_RETURN; break;
		case VK_MENU: id = IID_ALT; break;
		case VK_UP: id = IID_UPARROW; break;
		case VK_DOWN: id = IID_DOWNARROW; break;
		case VK_LEFT: id = IID_LEFTARROW; break;
		case VK_RIGHT: id = IID_RIGHTARROW; break;
		default: return;
		}

		bufferedData_[ nBufferData_ ].buttonID_ = id;
		bufferedData_[ nBufferData_ ].state_ = state ? IS_DOWN : IS_UP;
		nBufferData_ = (nBufferData_ + 1) % MAX_BUFFER_SIZE;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Keyboard::OnKeyPressEvent( hUint32 id, KernelEvents::KeyboardInputEvent& event )
	{
		if ( event->down_ )
		{
			keys_[ event->key_ ].raisingEdge_ = hTrue;
		}
		else //if ( !event->isRepeat_ )
		{
			keys_[ event->key_ ].fallingEdge_ = hTrue;
		}

		if ( event->isRepeat_ )
		{
			keys_[event->key_].isRepeat_ = hTrue;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Keyboard::OnCharacterEvent( hUint32 id, KernelEvents::KeyboardCharacterEvent& event )
	{
		hcWarningHigh( nCharacters_ == MAX_BUFFER_SIZE, "Keyboard buffer overflow" );

		charBuffer_[ nCharacters_ ] = event->char_;
		charBuffer_[ nCharacters_ + 1 ] = 0;
		nCharacters_ = (nCharacters_ + 1) % MAX_BUFFER_SIZE;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Keyboard::OnMouseEvent( hUint32 id, KernelEvents::MouseMoveEvent& event )
	{
		nextMouseX_.anologueVal_ += event->xDelta_;
		nextMouseY_.anologueVal_ += event->yDelta_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32 Keyboard::AquireExclusiveAccess() const
	{
		return accessKey_ == 0 ? accessKey_ = (rand() % 8096) + 1 : 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Keyboard::ReleaseExclusiveAccess( hUint32 accesskey ) const
	{
		if ( accesskey  == accessKey_ )
		{
			accessKey_ = 0;
		}
	}

}
}
