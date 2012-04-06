/********************************************************************

	filename: 	hKeyboard.cpp	
	
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
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hKeyboard::hKeyboard() :
		pKeyboard_( NULL )
		,pButtonKeyboard_( NULL )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hKeyboard::~hKeyboard()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hBool hKeyboard::Initialise( Device::ICharacterInput* pKbImpl, Device::IInput* pButtonKeyboard )
	{
		pKeyboard_ = pKbImpl;
		pButtonKeyboard_ = pButtonKeyboard;

		return hTrue;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	const hChar* hKeyboard::GetBufferedText() const
	{
		if ( pKeyboard_ )
		{
			return pKeyboard_->GetCharBufferData();
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hBool hKeyboard::GetButtonDown( Device::InputID id ) const
	{
		if ( pButtonKeyboard_ )
		{
			return pButtonKeyboard_->GetButton( id ).raisingEdge_;
		}
		return hFalse;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hBool hKeyboard::GetButtonDown( Device::InputID id, hUint32 accessKey ) const
	{
		if ( pButtonKeyboard_ )
		{
			return pButtonKeyboard_->GetButton( id, accessKey ).raisingEdge_;
		}
		return hFalse;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hBool hKeyboard::GetButtonPressed( Device::InputID id ) const
	{
		if ( pButtonKeyboard_ )
		{
			return pButtonKeyboard_->GetButton( id ).buttonVal_;
		}
		return hFalse;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hBool hKeyboard::GetButtonPressed( Device::InputID id, hUint32 accessKey ) const
	{
		if ( pButtonKeyboard_ )
		{
			return pButtonKeyboard_->GetButton( id, accessKey ).buttonVal_;
		}
		return hFalse;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hBool hKeyboard::GetButtonUp( Device::InputID id ) const
	{
		if ( pButtonKeyboard_ )
		{
			return pButtonKeyboard_->GetButton( id ).fallingEdge_;
		}
		return hFalse;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hBool hKeyboard::GetButtonUp( Device::InputID id, hUint32 accessKey ) const
	{
		if ( pButtonKeyboard_ )
		{
			return pButtonKeyboard_->GetButton( id, accessKey ).fallingEdge_;
		}
		return hFalse;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Heart::Device::InputAxis hKeyboard::GetAxis( Device::InputID id ) const
	{
		if ( pButtonKeyboard_ )
		{
			return pButtonKeyboard_->GetAxis( id );
		}
		return Device::InputAxis();
	}
}