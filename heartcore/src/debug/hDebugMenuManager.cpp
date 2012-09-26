/********************************************************************

	filename: 	hDebugMenuManager.cpp	
	
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

namespace Heart
{

    hDebugMenuManager* hDebugMenuManager::instance_ = NULL;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hDebugMenuManager::hDebugMenuManager()
        : renderer_(NULL)
        , resourceManager_(NULL)
        , uiRenderer_(NULL)
        , uiCanvas_(NULL)
    {
        hcAssert(instance_ == NULL);
        instance_ = this;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hDebugMenuManager::~hDebugMenuManager()
    {
        instance_ = NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugMenuManager::Initialise(hGwenRenderer* uirenderer,  hRenderer* renderer, hResourceManager* resManager, hControllerManager* controllerManager )
    {
        renderer_ = renderer;
        resourceManager_ = resManager;
        controllerManager_ = controllerManager;
        uiRenderer_ = uirenderer;

        menuMap_.SetHeap(GetDebugHeap());
        menuMap_.SetAutoDelete(hTrue);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugMenuManager::Destroy()
    {
        menuMap_.Clear(hTrue);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugMenuManager::RegisterMenu( const hChar* name, hDebugMenuBase* menu )
    {
        hcAssertMsg( menu->GetParent() == uiCanvas_, "Debug menus must has the debug ui canvas as a parent");
        menuMap_.Insert(hCRC32::StringCRC(name), menu);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugMenuManager::UnregisterMenu( hDebugMenuBase* menu )
    {
        if (!menu)
        {
            return;
        }

        menuMap_.Remove(menu);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugMenuManager::SetMenuVisiablity( const hChar* name, hBool show )
    {
        hDebugMenuBase* dm = menuMap_.Find(hCRC32::StringCRC(name));
        if (dm)
        {
            dm->SetHidden(!show);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugMenuManager::PreRenderUpdate()
    {
        for (hDebugMenuBase* dm = menuMap_.GetHead(); dm; dm = dm->GetNext())
        {
            dm->PreRenderUpdate();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugMenuManager::RenderMenus()
    {
        HEART_PROFILE_FUNC();
        if (resourceManager_->RequiredResourcesReady())
        {
            if (uiCanvas_ == NULL)
            {
                skin_.SetRender(uiRenderer_);
                skin_.Init("CORE.UISKIN");
                skin_.SetDefaultFont(Gwen::Utility::StringToUnicode("CORE.CONSOLE"));
                uiCanvas_ = hNEW(GetDebugHeap(), Gwen::Controls::Canvas)(&skin_);
                uiCanvas_->SetBackgroundColor(Gwen::Color(0, 255, 255, 64));
                uiCanvas_->SetDrawBackground(false);
                uiCanvas_->SetSize(renderer_->GetWidth(), renderer_->GetHeight());
                uiCanvas_->SetPos(0, 0);

                inputBinder_.SetCanvas(controllerManager_, uiCanvas_);
            }

            inputBinder_.Update();
            uiRenderer_->SetRenderCameraID(HEART_DEBUGUI_CAMERA_ID);//TODO: do this better...
            uiCanvas_->RenderCanvas();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugMenuManager::EndFrameUpdate()
    {
        for (hDebugMenuBase* dm = menuMap_.GetHead(); dm; dm = dm->GetNext())
        {
            dm->EndFrameUpdate();
        }
    }

}