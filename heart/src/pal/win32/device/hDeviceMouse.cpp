/********************************************************************

	filename: 	hDeviceMouse.cpp	
	
	Copyright (c) 11:8:2012 James Moran
	
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

void hdMouse::SetMousePosition( hFloat x, hFloat y )
{
    dx_ = x - absX_;
    dy_ = y - absY_;
    absX_ = x;
    absY_ = y;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Heart::hdInputButton hdMouse::GetButton( hdInputID buttonId ) const
{
    if (buttonId < HEART_MOUSE_BUTTON1 || buttonId > HEART_MOUSE_BUTTON5)
        return hdInputButton();

    return mb_[buttonId-HEART_MOUSE_BUTTON1];
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hdMouse::SetButton( hdInputID buttonID, hdInputButtonState state )
{
    hcAssert(!(buttonID < HEART_MOUSE_BUTTON1 || buttonID > HEART_MOUSE_BUTTON5));

    buttonID -= HEART_MOUSE_BUTTON1;
    mb_[buttonID].isRepeat_    = mb_[buttonID].buttonVal_;
    mb_[buttonID].raisingEdge_ = state == hButtonState_IS_DOWN;
    mb_[buttonID].fallingEdge_ = state == hButtonState_IS_UP;
    mb_[buttonID].buttonVal_   = state == hButtonState_IS_DOWN;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hdMouse::EndOfFrameUpdate()
{
    for ( hUint32 i = 0; i < buttonCount; ++i )
    {
        mb_[i].raisingEdge_ = hFalse;
        mb_[i].fallingEdge_ = hFalse;
        mb_[i].isRepeat_    = hFalse;
    }
}

}