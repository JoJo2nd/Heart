/********************************************************************

	filename: 	hDebugMenuManager.h	
	
	Copyright (c) 12:8:2012 James Moran
	
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
#ifndef HDEBUGMENUMANAGER_H__
#define HDEBUGMENUMANAGER_H__

namespace Heart
{
    class hDebugMenuBase : public Gwen::Controls::WindowControl
    {
    public:
        hDebugMenuBase(Gwen::Controls::Base* parent)
            : WindowControl(parent)
        {

        }

        virtual void PreRenderUpdate() = 0;
        virtual void EndFrameUpdate() = 0;
    };

    class hDebugMenuManager
    {
    public:

        hDebugMenuManager();
        ~hDebugMenuManager();

        void                        Initialise(hGwenRenderer* uirenderer, hRenderer* renderer, hResourceManager* resManager, hControllerManager* controllerManager);
        void                        Destroy();
        Gwen::Controls::Canvas*     GetDebugCanvas() const { hcAssertMsg(uiCanvas_, "Debug MenuManager is not Ready()"); return uiCanvas_; }
        void                        RegisterMenu(const hChar* name, Gwen::Controls::Base* menu);
        void                        UnregisterMenu(Gwen::Controls::Base* menu);
        void                        SetMenuVisiablity(const hChar* name, hBool show);
        void                        PreRenderUpdate() {}
        void                        RenderMenus();
        void                        EndFrameUpdate() {}
        static hDebugMenuManager*   GetInstance() { return instance_; }
        hBool                       Ready() { return uiCanvas_ != NULL; }

    private:

        struct hDebugMenu : public hMapElement< hUint32, hDebugMenu >
        {
            Gwen::Controls::Base* menu_;
        };

        typedef hMap< hUint32, hDebugMenu > hMenuMap;

        static hDebugMenuManager*   instance_;
        hRenderer*                  renderer_;
        hResourceManager*           resourceManager_;
        hControllerManager*         controllerManager_;
        hGwenRenderer*              uiRenderer_;
        hGwenInputBinder            inputBinder_;
        Gwen::Skin::TexturedBase    skin_;
        Gwen::Controls::Canvas*     uiCanvas_;
        hMenuMap                    menuMap_;
    };
}

#endif // HDEBUGMENUMANAGER_H__