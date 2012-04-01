/********************************************************************

	filename: 	hController.h	
	
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

#ifndef HOCONTROLLER_H__
#define HOCONTROLLER_H__

class hSystem;

namespace Heart
{
namespace Device
{
	class IInput;
}

namespace Private
{
	enum ButtonType
	{
		BT_Button,
		BT_Trigger,
		BT_Stick,
	};

	struct Button
	{
		Button() : buttonID_( hErrorCode )
		{

		}

		hUint32					buttonID_;
		ButtonType				type_;
		union
		{
			struct
			{
				hBool			buttonVal_;
				hBool			raisingEdge_;
				hBool			fallingEdge_;
			};
			hFloat				stickVal_[ 2 ];
			hFloat				anologueVal_;
		};
	};
}

	class ControllerConfig
	{
	public:

		void					SetButtonMapping( hUint32 realID, hUint32 buttonID );

	private:

		typedef hVector< Device::InputDeviceTypes >	ITypesVector;
		typedef hVector< Private::Button >          ButtonsVector;

		ITypesVector			devices_;
		ButtonsVector			buttonMappings_;	
		//config
	};

	class hController
	{
	public:

		static const hUint32 MAX_BUTTON_MAPPINGS = 128;
		hController();
		~hController() {};

		void				SetPadPort( hUint32 port ) { padPort_ = port; }
		hUint32				GetPadPort() const { return padPort_; }
		hBool				GetButtonDown( hUint32 buttonID ) const
		{ 
			hcAssert( buttonMappings_[ buttonID ].type_ == Private::BT_Button );
			return buttonMappings_[ buttonID ].raisingEdge_;
		}
		hBool				GetButtonPushed( hUint32 buttonID ) const
		{
			hcAssert( buttonMappings_[ buttonID ].type_ == Private::BT_Button );
			return buttonMappings_[ buttonID ].buttonVal_;
		}
		hBool				GetButtonUp( hUint32 buttonID ) const
		{
			hcAssert( buttonMappings_[ buttonID ].type_ == Private::BT_Button );
			return buttonMappings_[ buttonID ].fallingEdge_;
		}
		hFloat				GetButtonValue( hUint32 buttonID ) const
		{
			hcAssert( buttonMappings_[ buttonID ].type_ == Private::BT_Trigger || buttonMappings_[ buttonID ].type_ == Private::BT_Button );
			if( buttonMappings_[ buttonID ].type_ == Private::BT_Button )
			{
				return (hFloat)buttonMappings_[ buttonID ].buttonVal_;
			}
			else
			{
				return buttonMappings_[ buttonID ].anologueVal_;
			}
		}
		hVec2				GetStickValue( hUint32 buttonID )
		{
			hcAssert( buttonMappings_[ buttonID ].type_ == Private::BT_Stick );
			return hVec2( buttonMappings_[ buttonID ].stickVal_[ 0 ], buttonMappings_[ buttonID ].stickVal_[ 1 ] );
		}

		void				SetButtonMapping( hUint32 realID, hUint32 buttonID );

		void				Update();

	private:

		friend class hControllerManager;

		static int					mappingCompare( const void*, const void* );
#if 0
		void						GetButtonState( XINPUT_STATE &padState, hUint32 xid, hUint32 bid ) 
		{
			if ( padState.Gamepad.wButtons & xid && physicalButtons_[ bid ].buttonVal_ == hFalse )
			{
				physicalButtons_[ bid ].buttonVal_ = hTrue;
				physicalButtons_[ bid ].raisingEdge_ = hTrue;
			}
			else if ( padState.Gamepad.wButtons & xid && 
				physicalButtons_[ bid ].buttonVal_ == hTrue && 
				physicalButtons_[ bid ].raisingEdge_ == hTrue )
			{
				physicalButtons_[ bid ].raisingEdge_ = hFalse;
			}
			else if ( !(padState.Gamepad.wButtons & xid) && physicalButtons_[ bid ].buttonVal_ == hTrue ) 
			{
				physicalButtons_[ bid ].buttonVal_ = hFalse;
				physicalButtons_[ bid ].fallingEdge_ = hTrue;
				physicalButtons_[ bid ].raisingEdge_ = hFalse;
			}
			else if ( !(padState.Gamepad.wButtons & xid) && 
				physicalButtons_[ bid ].buttonVal_ == hFalse && 
				physicalButtons_[ bid ].fallingEdge_ == hTrue )
			{
				physicalButtons_[ bid ].fallingEdge_ = hFalse;
				physicalButtons_[ bid ].raisingEdge_ = hFalse;
			}
		}

		void						GetButtonStateKB( hBool keyState, hUint32 bid ) 
		{
			if ( keyState && physicalButtons_[ bid ].buttonVal_ == hFalse )
			{
				physicalButtons_[ bid ].buttonVal_ = hTrue;
				physicalButtons_[ bid ].raisingEdge_ = hTrue;
			}
			else if ( keyState && 
				physicalButtons_[ bid ].buttonVal_ == hTrue && 
				physicalButtons_[ bid ].raisingEdge_ == hTrue )
			{
				physicalButtons_[ bid ].raisingEdge_ = hFalse;
			}
			else if ( !(keyState) && physicalButtons_[ bid ].buttonVal_ == hTrue ) 
			{
				physicalButtons_[ bid ].buttonVal_ = hFalse;
				physicalButtons_[ bid ].fallingEdge_ = hTrue;
				physicalButtons_[ bid ].raisingEdge_ = hFalse;
			}
			else if ( !(keyState) && 
				physicalButtons_[ bid ].buttonVal_ == hFalse && 
				physicalButtons_[ bid ].fallingEdge_ == hTrue )
			{
				physicalButtons_[ bid ].fallingEdge_ = hFalse;
				physicalButtons_[ bid ].raisingEdge_ = hFalse;
			}
		}
#endif // 0

		hUint32						padPort_;
		hUint32						nButtonMappings_;//to remove
		Private::Button				buttonMappings_[ MAX_BUTTON_MAPPINGS ];//to remove

		ControllerConfig			padConfig_;
		
	};
}

#endif // HOCONTROLLER_H__