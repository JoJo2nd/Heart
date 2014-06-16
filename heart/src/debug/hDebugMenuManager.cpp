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
#if 0
#include "debug/hDebugMenuManager.h"
#include "base/hCRC32.h"
#include "base/hProfiler.h"
#include "input/hActionManager.h"
#include "render/hTexture.h"
#include "render/hViews.h"
#include "render/hRenderer.h"
#include "render/hRenderUtility.h"
#include "render/hRenderSubmissionContext.h"
#include "render/hRenderMaterialManager.h"

namespace Heart
{

    hDebugMenuManager* hDebugMenuManager::instance_ = NULL;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hDebugMenuManager::hDebugMenuManager()
        : renderer_(NULL)
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

    void hDebugMenuManager::Initialise(hRenderer* renderer, hActionManager* actionManager)
    {
        renderer_ = renderer;
        actionManager_ = actionManager;

        menuMap_.SetAutoDelete(hTrue);

        hRenderMaterialManager* matMgr=renderer->GetMaterialManager();
        hUint32 w = renderer->GetWidth();
        hUint32 h = renderer->GetHeight();
        hFloat aspect = (hFloat)w/(hFloat)h;
        hRenderViewportTargetSetup rtDesc={0};
        hTexture* bb=matMgr->getGlobalTexture("back_buffer");
        hRenderTargetView* rtv=NULL;
        hRenderTargetViewDesc rtvd;
        hDepthStencilViewDesc dsvd;
        hZeroMem(&rtvd, sizeof(rtvd));
        hZeroMem(&dsvd, sizeof(dsvd));
        rtvd.format_=bb->getTextureFormat();
        rtvd.resourceType_=bb->getRenderType();
        hcAssert(bb->getRenderType()==eRenderResourceType_Tex2D);
        rtvd.tex2D_.topMip_=0;
        rtvd.tex2D_.mipLevels_=~0;
        renderer->createRenderTargetView(bb, rtvd, &rtv);
        rtDesc.nTargets_=1;
        rtDesc.targetTex_=bb;
        rtDesc.targets_[0]=rtv;
        rtDesc.depth_=hNullptr;

        hRelativeViewport vp;
        vp.x= 0.f;
        vp.y= 0.f;
        vp.w= 1.f;
        vp.h= 1.f;
        Heart::hMatrix vm = Heart::hMatrix::identity();
        camera_.Initialise(renderer);
        camera_.bindRenderTargetSetup(rtDesc);
        camera_.SetFieldOfView(45.f);
        camera_.SetOrthoParams(0.f, 1.f, 2.f, -1.f, 0.f, 1000.f);
        camera_.SetViewMatrix(vm);
        camera_.setViewport(vp);
        camera_.SetTechniquePass(renderer->GetMaterialManager()->GetRenderTechniqueInfo("main"));

        rtv->DecRef();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugMenuManager::Destroy()
    {
        camera_.releaseRenderTargetSetup();
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

        //hMatrix view, proj;
        hViewport viewport;
        hRenderBuffer* camCB = matManager->GetGlobalConstantBlock(hCRC32::StringCRC("CameraConstants"));
        hRenderBuffer* instCB = matManager->GetGlobalConstantBlock(hCRC32::StringCRC("InstanceConstants"));
        hcAssertMsg(camCB, "Couldn't find global constant block \"CameraConstants\"");
        hcAssertMsg(instCB, "Couldn't find global constant block \"InstanceConstants\"");

        camera_.SetOrthoParams((hFloat)renderer_->GetWidth(), (hFloat)renderer_->GetHeight(), 0.f, 100.f);
        hRenderUtility::setCameraParameters(rndCtx, &camera_);

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
#endif