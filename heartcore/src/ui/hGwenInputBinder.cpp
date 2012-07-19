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
        if ( !canvas_ || !keyboard_ )
            return;

        hFloat newX = 0;//TODO:FIX keyboard_->GetAxis(Device::IID_MOUSEXAXIS).anologueVal_;
        hFloat newY = 0;//TODO:FIX keyboard_->GetAxis(Device::IID_MOUSEYAXIS).anologueVal_;

        //if (newX != mouseX_ || newY != mouseY_)
        {
            mouseX_ = Math::Util::Clamp(mouseX_-newX, (hFloat)canvas_->GetBounds().x, (hFloat)canvas_->GetBounds().w);
            mouseY_ = Math::Util::Clamp(mouseY_-newY, (hFloat)canvas_->GetBounds().y, (hFloat)canvas_->GetBounds().h);
            canvas_->InputMouseMoved(mouseX_, mouseY_, -newX, -newY);
        }

#if 0
        canvas_->InputMouseWheel(keyboard_->GetAxis(Device::IID_MOUSEWHEEL).anologueVal_);

        if (keyboard_->GetButtonDown(Device::IID_LEFTMOUSEBUTTON))
        {
            canvas_->InputMouseButton(0,true);
        }
        if (keyboard_->GetButtonDown(Device::IID_RIGHTMOUSEBUTTON))
        {
            canvas_->InputMouseButton(1,true);
        }
        if (keyboard_->GetButtonDown(Device::IID_MIDDLEMOUSEBUTTON))
        {
            canvas_->InputMouseButton(2,true);
        }

        if (keyboard_->GetButtonUp(Device::IID_LEFTMOUSEBUTTON))
        {
            canvas_->InputMouseButton(0,false);
        }
        if (keyboard_->GetButtonUp(Device::IID_RIGHTMOUSEBUTTON))
        {
            canvas_->InputMouseButton(1,false);
        }
        if (keyboard_->GetButtonUp(Device::IID_MIDDLEMOUSEBUTTON))
        {
            canvas_->InputMouseButton(2,false);
        }
#endif

        const hChar* str = keyboard_->GetCharBufferData();
        for(; *str; ++str)
        {
            canvas_->InputCharacter(*str);
        }

#if 0
        for (hUint32 i = 0; i < keyboard_->GetBufferKeysSize(); ++i)
        {
            switch(keyboard_->GetBufferedKeys()[i].buttonID_)
            {
            case Device::IID_LEFTCTRL: canvas_->InputKey(Gwen::Key::Control,keyboard_->GetBufferedKeys()[i].state_ == Device::IS_DOWN); break;
            case Device::IID_RIGHTCTRL: canvas_->InputKey(Gwen::Key::Control,keyboard_->GetBufferedKeys()[i].state_ == Device::IS_DOWN); break;
            case Device::IID_LEFTSHIFT: canvas_->InputKey(Gwen::Key::Shift,keyboard_->GetBufferedKeys()[i].state_ == Device::IS_DOWN); break;
            case Device::IID_RIGHTSHIFT: canvas_->InputKey(Gwen::Key::Shift,keyboard_->GetBufferedKeys()[i].state_ == Device::IS_DOWN); break;
            case Device::IID_TAB: canvas_->InputKey(Gwen::Key::Tab,keyboard_->GetBufferedKeys()[i].state_ == Device::IS_DOWN); break;
            case Device::IID_RETURN: canvas_->InputKey(Gwen::Key::Return,keyboard_->GetBufferedKeys()[i].state_ == Device::IS_DOWN); break;
            case Device::IID_ALT: canvas_->InputKey(Gwen::Key::Alt,keyboard_->GetBufferedKeys()[i].state_ == Device::IS_DOWN); break;
            case Device::IID_UPARROW: canvas_->InputKey(Gwen::Key::Up,keyboard_->GetBufferedKeys()[i].state_ == Device::IS_DOWN); break;
            case Device::IID_DOWNARROW: canvas_->InputKey(Gwen::Key::Down,keyboard_->GetBufferedKeys()[i].state_ == Device::IS_DOWN); break;
            case Device::IID_LEFTARROW: canvas_->InputKey(Gwen::Key::Left,keyboard_->GetBufferedKeys()[i].state_ == Device::IS_DOWN); break;
            case Device::IID_RIGHTARROW: canvas_->InputKey(Gwen::Key::Right,keyboard_->GetBufferedKeys()[i].state_ == Device::IS_DOWN); break;
            case Device::IID_DELETE: canvas_->InputKey(Gwen::Key::Delete,keyboard_->GetBufferedKeys()[i].state_ == Device::IS_DOWN); break;
            case Device::IID_BACKSPACE: canvas_->InputKey(Gwen::Key::Backspace,keyboard_->GetBufferedKeys()[i].state_ == Device::IS_DOWN); break;
            default:
                break;
            }
        }
#endif
    }

}