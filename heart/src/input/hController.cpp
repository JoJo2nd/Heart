/********************************************************************

	filename: 	hController.cpp	
	
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


namespace Heart
{
	hController::hController() :
		nButtonMappings_( 0 ),
		padPort_( hErrorCode )
	{
// 		for ( hUint32 i = 0; i < XPAD_MAX; ++i )
// 		{
// 			Button& button = physicalButtons_[ i ];
// 			button.buttonID_ = i;
// 			switch( i )
// 			{
// 			case MOUSE_AS_STICK:
// 			case XPAD_RIGHT_STICK:
// 			case XPAD_LEFT_STICK:
// 				button.type_ = BT_Stick;
// 				break;
// 			case XPAD_LEFT_TRIGGER:
// 			case XPAD_RIGHT_TRIGGER:
// 				button.type_ = BT_Trigger;
// 				break;
// 			default:
// 				button.type_ = BT_Button;
// 				break;
// 			}
// 		}
// 
// 		XInputEnable( TRUE );
// 
// 		ShowCursor( FALSE );
	}

	int hController::mappingCompare( const void* a, const void* b )
	{
		return ((Private::Button*)a)->buttonID_ > ((Private::Button*)b)->buttonID_;
	}

	void hController::SetButtonMapping( hUint32 realID, hUint32 buttonID )
	{
		nButtonMappings_++;
// 		Button& button = buttonMappings_[ buttonID ];
// 		button.buttonID_ = realID;
// 		switch( realID )
// 		{
// 		case MOUSE_AS_STICK:
// 		case XPAD_RIGHT_STICK:
// 		case XPAD_LEFT_STICK:
// 			button.type_ = BT_Stick;
// 			break;
// 		case XPAD_LEFT_TRIGGER:
// 		case XPAD_RIGHT_TRIGGER:
// 			button.type_ = BT_Trigger;
// 			break;
// 		default:
// 			button.type_ = BT_Button;
// 			break;
// 		}
	}

	hFloat calcValStick( hFloat val, hFloat min, hFloat max, hFloat deadzone )
	{
		hFloat aval = fabs( val );

		if ( aval < deadzone )
		{
			return 0.0f;
		}
		else
		{
			hFloat range = ( max - min ) * 0.5f - deadzone;

			aval -= deadzone;
			if ( val < 0.0f )
			{
				aval = -aval;
			}

			aval /= range;

			if ( aval < -1.0f )
			{
				aval = -1.0f;
			}

			if ( aval > 1.0f )
			{
				aval = 1.0f;
			}

			return aval;
		}
	}

	hFloat calcVal( hFloat val, hFloat min, hFloat max, hFloat deadzone )
	{
		hFloat aval = fabs( val );

		if ( aval < deadzone )
		{
			return 0.0f;
		}
		else
		{
			hFloat range = ( max - min ) - deadzone;

			aval -= deadzone;
			if ( val < 0.0f )
			{
				aval = -aval;
			}

			aval /= range;

			if ( aval < -1.0f )
			{
				aval = -1.0f;
			}

			if ( aval > 1.0f )
			{
				aval = 1.0f;
			}

			return aval;
		}
	}

	void hController::Update()
	{
#if 0
		if ( padPort_ == hErrorCode )
		{
			return;
		}

		//poll the XINPUT pad
		XINPUT_STATE padState;
		ZeroMemory( &padState, sizeof(XINPUT_STATE) );
		HRESULT hr = XInputGetState( padPort_, &padState );

		if ( hr == S_OK )
		{
			physicalButtons_[ XPAD_LEFT_STICK ].stickVal_[ 0 ] = calcValStick( (hFloat)padState.Gamepad.sThumbLX, (hInt16)0x8000, (hInt16)0x7FFF, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );
			physicalButtons_[ XPAD_LEFT_STICK ].stickVal_[ 1 ] = calcValStick( (hFloat)padState.Gamepad.sThumbLY, (hInt16)0x8000, (hInt16)0x7FFF, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );

			physicalButtons_[ XPAD_RIGHT_STICK ].stickVal_[ 0 ] = calcValStick( (hFloat)padState.Gamepad.sThumbRX, (hInt16)0x8000, (hInt16)0x7FFF, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );
			physicalButtons_[ XPAD_RIGHT_STICK ].stickVal_[ 1 ] = calcValStick( (hFloat)padState.Gamepad.sThumbRY, (hInt16)0x8000, (hInt16)0x7FFF, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );

			physicalButtons_[ XPAD_LEFT_TRIGGER ].anologueVal_  = calcVal( (hFloat)padState.Gamepad.bLeftTrigger, 0, 0xFF, XINPUT_GAMEPAD_TRIGGER_THRESHOLD );
			physicalButtons_[ XPAD_RIGHT_TRIGGER ].anologueVal_ = calcVal( (hFloat)padState.Gamepad.bRightTrigger, 0, 0xFF, XINPUT_GAMEPAD_TRIGGER_THRESHOLD );

			//buttons
			GetButtonState(padState, XINPUT_GAMEPAD_DPAD_UP,		XPAD_DPAD_UP );
			GetButtonState(padState, XINPUT_GAMEPAD_DPAD_DOWN,		XPAD_DPAD_DOWN );
			GetButtonState(padState, XINPUT_GAMEPAD_DPAD_LEFT,		XPAD_DPAD_LEFT );
			GetButtonState(padState, XINPUT_GAMEPAD_DPAD_RIGHT, 	XPAD_DPAD_RIGHT );
			GetButtonState(padState, XINPUT_GAMEPAD_A,				XPAD_A_BUTTON );
			GetButtonState(padState, XINPUT_GAMEPAD_B,				XPAD_B_BUTTON );
			GetButtonState(padState, XINPUT_GAMEPAD_X,				XPAD_X_BUTTON );
			GetButtonState(padState, XINPUT_GAMEPAD_Y,				XPAD_Y_BUTTON );
			GetButtonState(padState, XINPUT_GAMEPAD_LEFT_SHOULDER,	XPAD_LEFT_BUMPER );
			GetButtonState(padState, XINPUT_GAMEPAD_RIGHT_SHOULDER, XPAD_RIGHT_BUMPER );
			GetButtonState(padState, XINPUT_GAMEPAD_BACK,			XPAD_BACK_BUTTON );
			GetButtonState(padState, XINPUT_GAMEPAD_START,			XPAD_START_BUTTON );
		}

		for ( hUint32 i = 0; i < 256; ++i )
		{
	// 		if( i == KEY_SPACE ) 
	// 		{
	// 			GetButtonStateKB( keyboardState_[ i ], i );
	// 		}
	// 		else if ( i == KEY_RETURN )
	// 		{
	// 			GetButtonStateKB( keyboardState_[ i ], i );
	// 		}
	// 		else if ( ( i >= KEYMAPPING_START && i <= KEYMAPPING_END ) || 
	// 				( i >= '1' && i <= '0' ) )
	// 		{
				GetButtonStateKB( keyboardState_[ i ], i );
	//		}
		}

		if ( gettingMouseInput_ )
		{
			physicalButtons_[ MOUSE_AS_STICK ].stickVal_[ 0 ] = calcValStick( (hFloat)mouseXChange_, -2.0f, 2.0f, 0.0f );
			physicalButtons_[ MOUSE_AS_STICK ].stickVal_[ 1 ] = calcValStick( (hFloat)mouseYChange_, -2.0f, 2.0f, 0.0f );

			mouseXChange_ = 0;
			mouseYChange_ = 0;

			RECT wndRect;
			RECT cltRect;
			GetWindowRect( hWnd_, &wndRect );
			GetClientRect( hWnd_, &cltRect );

			SetCursorPos( wndRect.left + (wndRect.right - wndRect.left) / 2, wndRect.top + (wndRect.bottom - wndRect.top) / 2 );
			POINT curPos;
			GetCursorPos( &curPos );
			
			mouseX_ = (short)curPos.x;
			mouseY_ = (short)curPos.y;

		}
#endif // 0

		//fill in the button mappings
		for ( hUint32 i = 0; i < nButtonMappings_; ++i )
		{
			Private::Button& b = buttonMappings_[ i ];
// 			Button& pb = physicalButtons_[ b.buttonID_ ];
// 			b = pb;
		}
	}
}