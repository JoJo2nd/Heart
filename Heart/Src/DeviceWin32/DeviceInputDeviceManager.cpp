/********************************************************************

	filename: 	DeviceInputDeviceManager.cpp	
	
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

	InputDeviceManager::InputDeviceManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	InputDeviceManager::~InputDeviceManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hBool InputDeviceManager::Initialise( EventManager* pEvManager )
	{
		keyboard_.Initialise( 0, pEvManager );
	//	mouse_.Initialise( 0, pEvManager );

		return hTrue;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void InputDeviceManager::Destory()
	{
		keyboard_.Destroy();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	IInput* InputDeviceManager::GetInputDevice( InputDeviceTypes type, hUint32 port )
	{
		switch( type )
		{
		case IDT_KEYBOARD:
			{
				if ( port == 0 || port == HEART_INPUT_ANY_PORT )
				{
					return &keyboard_;
				}
				else 
				{
					return NULL;
				}
			}
			break;
		default:
			return NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	ICharacterInput* InputDeviceManager::GetCharacterInputDevice( InputDeviceTypes type, hUint32 port )
	{
		switch( type )
		{
		case IDT_KEYBOARD:
			{
				if ( port == 0 || port == HEART_INPUT_ANY_PORT )
				{
					return &keyboard_;
				}
				else 
				{
					return NULL;
				}
			}
			break;
		default:
			return NULL;
		}
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    IBufferedInput* InputDeviceManager::GetBufferInputDevice( InputDeviceTypes type, hUint32 port )
    {
        switch( type )
        {
        case IDT_KEYBOARD:
            {
                if ( port == 0 || port == HEART_INPUT_ANY_PORT )
                {
                    return &keyboard_;
                }
                else 
                {
                    return NULL;
                }
            }
            break;
        default:
            return NULL;
        }
    }


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void InputDeviceManager::Update()
	{
		keyboard_.Update();
	}

}
}