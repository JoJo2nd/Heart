/********************************************************************

	filename: 	hGwenInputBinder.cpp	
	
	Copyright (c) 6:5:2012 James Moran
	
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

    void hGwenInputBinder::Update()
    {
        if ( !canvas_ || !keyboard_ || !mouse_ )
            return;

        canvas_->InputMouseMoved(mouse_->GetAbsoluteX(), mouse_->GetAbsoluteY(), 0, 0);
        //canvas_->InputMouseMoved(mouse_->GetAbsoluteX(), mouse_->GetAbsoluteY(), -mouse_->GetDeltaX(), -mouse_->GetDeltaX());

        //canvas_->InputMouseWheel(keyboard_->GetAxis(Device::IID_MOUSEWHEEL).anologueVal_);

        canvas_->InputMouseButton(0,mouse_->GetButton(HEART_MOUSE_BUTTON1).buttonVal_);
        canvas_->InputMouseButton(1,mouse_->GetButton(HEART_MOUSE_BUTTON2).buttonVal_);
        canvas_->InputMouseButton(2,mouse_->GetButton(HEART_MOUSE_BUTTON3).buttonVal_);

        const hChar* str = keyboard_->GetCharBufferData();
        for(; *str; ++str)
        {
            canvas_->InputCharacter(*str);
        }

        canvas_->InputKey(Gwen::Key::Control,keyboard_->GetButton(VK_CONTROL).buttonVal_);
        canvas_->InputKey(Gwen::Key::Shift,keyboard_->GetButton(VK_LSHIFT).buttonVal_);
        canvas_->InputKey(Gwen::Key::Shift,keyboard_->GetButton(VK_RSHIFT).buttonVal_);
        canvas_->InputKey(Gwen::Key::Tab,keyboard_->GetButton(VK_TAB).buttonVal_);
        canvas_->InputKey(Gwen::Key::Return,keyboard_->GetButton(VK_RETURN).buttonVal_);
        canvas_->InputKey(Gwen::Key::Alt,keyboard_->GetButton(VK_RMENU).buttonVal_);
        canvas_->InputKey(Gwen::Key::Alt,keyboard_->GetButton(VK_LMENU).buttonVal_);
        canvas_->InputKey(Gwen::Key::Up,keyboard_->GetButton(VK_UP).buttonVal_);
        canvas_->InputKey(Gwen::Key::Down,keyboard_->GetButton(VK_DOWN).buttonVal_);
        canvas_->InputKey(Gwen::Key::Left,keyboard_->GetButton(VK_LEFT).buttonVal_);
        canvas_->InputKey(Gwen::Key::Right,keyboard_->GetButton(VK_RIGHT).buttonVal_);
        canvas_->InputKey(Gwen::Key::Delete,keyboard_->GetButton(VK_DELETE).buttonVal_);
        canvas_->InputKey(Gwen::Key::Backspace,keyboard_->GetButton(VK_BACK).buttonVal_);
        canvas_->InputKey(Gwen::Key::Home,keyboard_->GetButton(VK_HOME).buttonVal_);
        canvas_->InputKey(Gwen::Key::End,keyboard_->GetButton(VK_END).buttonVal_);

    }

}