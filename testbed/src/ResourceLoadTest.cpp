/********************************************************************

	filename: 	ResourceLoadTest.cpp	
	
	Copyright (c) 3:12:2011 James Moran
	
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

#include "ResourceLoadTest.h"
#include "Gwen/UnitTest/UnitTest.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ResourceLoadTest::PreEnter()
{
    resPack_.AddResourceToPackage("TEXTURES/TEST_TEXTURE_MAP.TEX", engine_->GetResourceManager());
    resPack_.AddResourceToPackage("TEXTURES/NARUTO_TEST.TEX", engine_->GetResourceManager());
    resPack_.AddResourceToPackage("EFFECTS/SIMPLE2.CFX", engine_->GetResourceManager());
    resPack_.AddResourceToPackage("MUSIC/CAFO_S48.OGG", engine_->GetResourceManager());
    resPack_.AddResourceToPackage("SFX/SNDBANK.SBK", engine_->GetResourceManager());
    resPack_.AddResourceToPackage("WORLD_OBJECTS.WOD", engine_->GetResourceManager());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 ResourceLoadTest::Enter()
{
    if ( resPack_.IsPackageLoaded() )
    {

        tex1_   = (Heart::hTexture*)resPack_.GetResource(0);
        tex2_   = (Heart::hTexture*)resPack_.GetResource(1);    
        materialResource_ = (Heart::hMaterial*)resPack_.GetResource(2);
        stream_ = (Heart::hSoundResource*)resPack_.GetResource(3);
        soundBank_ = (Heart::hSoundBankResource*)resPack_.GetResource(4);
        script_ = (Heart::hWorldScriptObject*)resPack_.GetResource(5);

        engine_->GetEntityFactory()->ActivateWorldScriptObject(script_);

        //Start playing a sound
        soundSource_ = engine_->GetSoundManager()->CreateSoundSource( 0 );
        soundSource_->SetSoundResource(stream_);
        soundSource_->SetLooping( hTrue );
        soundSource_->Start();

        for ( hUint32 i = 0; i < 11; ++i )
        {
            staticSource_[i] = engine_->GetSoundManager()->CreateSoundSource(0);
            staticSource_[i]->SetSoundResource( soundBank_->GetSoundSource(i) );
            staticSource_[i]->SetLooping( hTrue );
            //staticSource_[i]->Start();
            staticSource_[i]->SetVolume( 0.0f );
        }

        //Setup a view port for rendering
        Heart::hRenderViewportTargetSetup rtDesc;
        rtDesc.nTargets_ = 0;
        rtDesc.width_ = engine_->GetRenderer()->GetWidth();
        rtDesc.height_ = engine_->GetRenderer()->GetHeight();
        rtDesc.targetFormat_ = Heart::TFORMAT_ARGB8;
        rtDesc.hasDepthStencil_ = hFalse;
        rtDesc.depthFormat_ = Heart::TFORMAT_D24S8F;

        Heart::hViewport vp;
        vp.x_ = 0;
        vp.y_ = 0;
        vp.width_ = engine_->GetRenderer()->GetWidth();
        vp.height_ = engine_->GetRenderer()->GetHeight();

        viewport_.Initialise( engine_->GetRenderer() );
        viewport_.SetRenderTargetSetup( rtDesc );
        viewport_.SetFieldOfView( 45.f );
        viewport_.SetProjectionParams( engine_->GetRenderer()->GetRatio(), 1.f, 1000.f );
        //viewport_.SetOrthoParams(2.f, 2.f, 0.f, 20.f);
        viewport_.SetViewMatrix( Heart::hMatrixFunc::identity() );
        viewport_.SetViewport( vp );
        viewport_.SetTechniquePass(engine_->GetRenderer()->GetMaterialManager()->GetRenderTechniqueInfo("main"));

        hUint16 idx[] = 
        {
            0, 1, 2
        };

        hFloat vtx[] = 
        {
            -1.f, 0.f, 10.f,
            0.f,  1.f, 10.f,
            1.f, 0.f,  10.f,
        };

        engine_->GetRenderer()->CreateIndexBuffer( idx, 3, 0, Heart::PRIMITIVETYPE_TRILIST, &ib_ );
        engine_->GetRenderer()->CreateVertexBuffer( vtx, 3, Heart::hrVF_XYZ, 0, &vb_ );

        material_ = materialResource_->CreateMaterialInstance();
        rndCtx_ = engine_->GetRenderer()->CreateRenderSubmissionCtx();

        //UI test setup
        gwenInput_ = hNEW(Heart::GetGlobalHeap(),Heart::hGwenInputBinder);
        gwenSkin_ = hNEW(Heart::GetGlobalHeap(),Gwen::Skin::TexturedBase);
        gwenSkin_->SetRender(engine_->GetUIRenderer());
        gwenSkin_->Init("ENGINE/TEXTURES/UI_SKIN.TEX");
        gwenSkin_->SetDefaultFont(Gwen::Utility::StringToUnicode("ENGINE/FONTS/UI.FNT"));
        canvas_ = hNEW(Heart::GetGlobalHeap(),Gwen::Controls::Canvas)(gwenSkin_);
        canvas_->SetBackgroundColor(Gwen::Color(0, 255, 255, 64));
        canvas_->SetDrawBackground(true);
        canvas_->SetSize( engine_->GetRenderer()->GetWidth(), engine_->GetRenderer()->GetHeight() );

        UnitTest* pUnit = hNEW(Heart::GetGlobalHeap(), UnitTest)(canvas_);
        pUnit->SetPos( 10, 10 );

        gwenInput_->SetCanvas(engine_->GetControllerManager(),canvas_);

        return Heart::hStateBase::FINISHED;
    }
    
    return Heart::hStateBase::CONTINUE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 ResourceLoadTest::Main()
{
    if (gwenInput_)
    {
        gwenInput_->Update();
    }
    return Heart::hStateBase::CONTINUE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ResourceLoadTest::MainRender()
{

//    rndCtx_->BeginPIXDebugging();
//     rndCtx_->SetRenderTarget( 0, NULL/*viewport_.GetRenderTarget(0)*/ );
//     rndCtx_->SetDepthTarget( NULL/*viewport_.GetDepthTarget()*/ );
//     rndCtx_->EnableDebugDrawing( hTrue );
//     rndCtx_->SetRendererCamera( &viewport_ );
//     rndCtx_->SetViewport( viewport_.GetViewport() );

    rndCtx_->SetRendererCamera( &viewport_ );
    rndCtx_->SetMaterialInstance(material_);
    rndCtx_->SetWorldMatrix( Heart::hMatrixFunc::identity() );
    rndCtx_->ClearTarget( hTrue, Heart::hColour(.2f,.1f,.5f,1.f), hTrue, 1.f );
    rndCtx_->SetPrimitiveType( Heart::PRIMITIVETYPE_TRILIST );
    rndCtx_->SetVertexStream( 0, vb_ );
    rndCtx_->SetIndexStream( ib_ );

    hUint32 passes = rndCtx_->GetMaterialInstancePasses();
    for ( hUint32 i = 0; i < passes; ++i )
    {
        rndCtx_->BeingMaterialInstancePass( i );
        rndCtx_->DrawIndexedPrimitive( 1, 0 );
        rndCtx_->EndMaterialInstancePass();
    }

    Heart::hdRenderCommandBuffer cmdBuf = rndCtx_->SaveToCommandBuffer();

    engine_->GetRenderer()->SubmitRenderCommandBuffer( cmdBuf, hTrue );

    canvas_->RenderCanvas();

    engine_->GetUIRenderer()->SubmitCommandsToRenderer();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ResourceLoadTest::PreLeave()
{
    //UI Delete
    gwenSkin_->m_Texture.Release(gwenSkin_->GetRender());
    hDELETE_SAFE(Heart::GetGlobalHeap(), gwenInput_);
    hDELETE_SAFE(Heart::GetGlobalHeap(), canvas_);
    hDELETE_SAFE(Heart::GetGlobalHeap(), gwenSkin_);

    engine_->GetRenderer()->DestroyIndexBuffer(ib_);
    engine_->GetRenderer()->DestroyVertexBuffer(vb_);
    engine_->GetRenderer()->DestroyRenderSubmissionCtx(rndCtx_);

    materialResource_->DestroyMaterialInstance(material_);

    engine_->GetSoundManager()->DestroySoundSource(soundSource_);
    soundSource_ = NULL;
    for ( hUint32 i = 0; i < 11; ++i )
    {
        engine_->GetSoundManager()->DestroySoundSource(staticSource_[i]);
        staticSource_[i] = NULL;
    }

    engine_->GetEntityFactory()->DeactivateWorldScriptObject();
    HEART_RESOURCE_SAFE_RELEASE(tex1_);
    HEART_RESOURCE_SAFE_RELEASE(tex2_);
    HEART_RESOURCE_SAFE_RELEASE(font1_);
    HEART_RESOURCE_SAFE_RELEASE(materialResource_);
    HEART_RESOURCE_SAFE_RELEASE(stream_);
    HEART_RESOURCE_SAFE_RELEASE(soundBank_);
    HEART_RESOURCE_SAFE_RELEASE(script_);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 ResourceLoadTest::Leave()
{
    return Heart::hStateBase::FINISHED;
}
