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

    void hDebugMenuManager::Initialise(hRenderer* renderer, hResourceManager* resManager, hControllerManager* controllerManager)
    {
        renderer_ = renderer;
        resourceManager_ = resManager;
        controllerManager_ = controllerManager;

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
            dm->SetVisible(show);
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

    void hDebugMenuManager::RenderMenus(hRenderSubmissionCtx* rndCtx, hRenderMaterialManager* matManager)
    {
        HEART_PROFILE_FUNC();

        hConstBlockMapInfo camMap;
        hViewportShaderConstants* camData;
        hMatrix view, proj;
        hViewport viewport;
        hParameterConstantBlock* camCB = matManager->GetGlobalConstantBlock(hCRC32::StringCRC("CameraConstants"));
        hParameterConstantBlock* instCB = matManager->GetGlobalConstantBlock(hCRC32::StringCRC("InstanceConstants"));
        hcAssertMsg(camCB, "Couldn't find global constant block \"CameraConstants\"");
        hcAssertMsg(instCB, "Couldn't find global constant block \"InstanceConstants\"");

        rndCtx->Map(camCB, &camMap);
        camData = (hViewportShaderConstants*)camMap.ptr;

        viewport.x_ = 0;
        viewport.y_ = 0;
        viewport.width_ = renderer_->GetWidth();
        viewport.height_ = renderer_->GetHeight();
        view = hMatrixFunc::identity();
        proj = hMatrixFunc::orthoProj((hFloat)renderer_->GetWidth(), (hFloat)renderer_->GetHeight(), 0.f, 100.f);

        camData->projection_ = proj;
        camData->projectionInverse_ = hMatrixFunc::inverse(proj);
        camData->view_ = view;
        camData->viewInverse_ = hMatrixFunc::inverse( view );
        camData->viewInverseTranspose_ = hMatrixFunc::transpose( camData->viewInverse_ );
        camData->viewProjection_ = hMatrixFunc::mult(view, proj);
        camData->viewProjectionInverse_ = hMatrixFunc::inverse(camData->viewProjection_);
        camData->viewportSize_[0]=(hFloat)renderer_->GetWidth();
        camData->viewportSize_[1]=(hFloat)renderer_->GetHeight();
        camData->viewportSize_[2]=(hFloat)renderer_->GetWidth();
        camData->viewportSize_[3]=(hFloat)renderer_->GetHeight();
        rndCtx->Unmap(&camMap);

        rndCtx->SetViewport(viewport);

        hDebugRenderParams params;
        params.rtWidth_ = renderer_->GetWidth();
        params.rtHeight_ = renderer_->GetHeight();
        for (hDebugMenuBase* i = menuMap_.GetHead(); i != NULL; i = i->GetNext())
        {
            i->Render(rndCtx, instCB, params);
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