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

    typedef hUint32 hdInputID;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	enum hdInputButtonState
	{
		IS_UP,
		IS_DOWN = 1,
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

    struct HEARTDEV_SLIBEXPORT hdInputButton
	{
		hdInputButton() 
            : buttonVal_( hFalse )
			, raisingEdge_( hFalse )
			, fallingEdge_( hFalse )
		{}
		hBool			buttonVal_		: 1;
		hBool			raisingEdge_	: 1;
		hBool			fallingEdge_	: 1;
		hBool			isRepeat_		: 1;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	struct HEARTDEV_SLIBEXPORT hdInputAxis
	{
		hdInputAxis() 
            : anologueVal_( 0.0f ) 
		{}
		hFloat			anologueVal_;
	};
}

#endif // DEVICEIINPUT_H__