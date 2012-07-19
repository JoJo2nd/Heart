/********************************************************************

	filename: 	hGwenInputBinder.h	
	
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
#ifndef HGWENINPUTBINDER_H__
#define HGWENINPUTBINDER_H__

namespace Heart
{
    class HEARTCORE_SLIBEXPORT hGwenInputBinder
    {
    public:
        hGwenInputBinder()
            : manager_(NULL)
            , keyboard_(NULL)
            , canvas_(NULL)
        {

        }
        ~hGwenInputBinder()
        {

        }

        void SetCanvas(hControllerManager* manager, Gwen::Controls::Canvas* canvas)
        {
            manager_ = manager;
            keyboard_ = manager->GetSystemKeyboard();
            canvas_ = canvas;

            mouseX_ = 0;//TODO:FIXkeyboard_->GetAxis(Device::IID_MOUSEXAXIS).anologueVal_;
            mouseY_ = 0;//TODO:FIXkeyboard_->GetAxis(Device::IID_MOUSEYAXIS).anologueVal_;
        }
        void Update();

    private:

        hControllerManager*     manager_;
        const hdKeyboard*       keyboard_;
        Gwen::Controls::Canvas* canvas_;
        hFloat                  mouseX_;
        hFloat                  mouseY_;
    };
}

#endif // HGWENINPUTBINDER_H__