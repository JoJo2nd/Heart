#include "precompiled.h"
#include "simple_sprite.h"
#include "PlayerInputComponent.h"
#include "ObjectBase.h"

//#define FULLSCREEN

using namespace Heart;

Heart::hResourcePackage* g_initPackage = NULL;
hWorldScriptObject* g_levelScript = NULL;
hRenderSubmissionCtx* g_rndCtx = NULL;
Heart::hRendererCamera* g_viewCamera = NULL;

void Initialise(const hChar* pCmdLine, Heart::HeartEngine*);
void EngineUpdateTick(hFloat tick, Heart::HeartEngine*);
void EngineRenderTick(hFloat tick, Heart::HeartEngine*);
bool EngineShutdownRequest(Heart::HeartEngine*);
void EngineShutdown(Heart::HeartEngine*);


//huMiniDumper MiniDumper;// Comment this out if you dont want the engine to create crash dumps [10/8/2008 James]
Heart::HeartEngine*& pEngine = // Global ptr to the Heart Engine [10/8/2008 James]
	Heart::SetupHeartEngineConfigParams( 
		Heart::HeartConfig(
			3.0f,
			"Gamedata/GamedataPak.zip",
			Heart::PostEngineInit::bind< &Initialise >(),
			Heart::OnUpdate::bind< &EngineUpdateTick >(),
			Heart::OnRender::bind< &EngineRenderTick >(),
			Heart::OnShutdownTick::bind< &EngineShutdownRequest >(),
			Heart::PostEngineShutdown::bind< &EngineShutdown >()
		)
	);
	
int WINAPI WinMain( HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	return Heart::HeartMain( hinstance, hPrevInstance, lpCmdLine, nCmdShow );
}

void Initialise(const hChar* pCmdLine, Heart::HeartEngine* engine)
{
    HEART_REGISTER_COMPONENT_FACTORY(engine->GetEntityFactory(), SimpleSprite,
        Heart::ComponentCreateCallback::bind<&SimpleSprite::SimpleSpriteComponentCreate>(), 
        Heart::ComponentDestroyCallback::bind<&SimpleSprite::SimpleSpriteComponentDestroy>());
    PlayerInputComponent::SetControllerManager(engine->GetControllerManager());
    HEART_REGISTER_COMPONENT_FACTORY(engine->GetEntityFactory(), PlayerInputComponent,
        Heart::ComponentCreateCallback::bind<&PlayerInputComponent::PlayerInputComponentCreate>(),
        Heart::ComponentDestroyCallback::bind<&PlayerInputComponent::PlayerInputComponentDestroy>());
    HEART_REGISTER_COMPONENT_FACTORY(engine->GetEntityFactory(), ObjectBaseComponent,
        Heart::ComponentCreateCallback::bind<&ObjectBaseComponent::ObjectBaseComponentCreate>(),
        Heart::ComponentDestroyCallback::bind<&ObjectBaseComponent::ObjectBaseComponentDestroy>());

    g_viewCamera = hNEW(hGeneralHeap, hRendererCamera);

    g_initPackage = hNEW(hGeneralHeap, hResourcePackage);
    g_initPackage->AddResourceToPackage("WORLDOBJECT/WORLDOBJECTS.WOD", engine->GetResourceManager());

    g_rndCtx = engine->GetRenderer()->CreateRenderSubmissionCtx();

    Heart::hRenderViewportTargetSetup rtDesc;
    rtDesc.nTargets_ = 0;
    rtDesc.width_ = engine->GetRenderer()->GetWidth();
    rtDesc.height_ = engine->GetRenderer()->GetHeight();
    rtDesc.targetFormat_ = Heart::TFORMAT_ARGB8;
    rtDesc.hasDepthStencil_ = hFalse;
    rtDesc.depthFormat_ = Heart::TFORMAT_D24S8F;

    Heart::hViewport vp;
    vp.x_ = 0;
    vp.y_ = 0;
    vp.width_ = engine->GetRenderer()->GetWidth();
    vp.height_ = engine->GetRenderer()->GetHeight();

    g_viewCamera->Initialise( engine->GetRenderer() );
    g_viewCamera->SetRenderTargetSetup( rtDesc );
    g_viewCamera->SetFieldOfView( 45.f );
    //g_viewCamera.SetProjectionParams( engine->GetRenderer()->GetRatio(), 1.f, 1000.f );
    g_viewCamera->SetOrthoParams(engine->GetRenderer()->GetRatio(), 1.f, 0.f, 1.f);
    g_viewCamera->SetViewport( vp );
    g_viewCamera->SetTechniquePass( engine->GetRenderer()->GetMaterialManager()->GetRenderTechniqueInfo("main"));
}

void EngineUpdateTick(hFloat tick, Heart::HeartEngine* engine)
{
    if (g_initPackage->IsPackageLoaded() && !g_levelScript)
    {
        g_levelScript = static_cast<hWorldScriptObject*>(g_initPackage->GetResource(0));

        engine->GetEntityFactory()->ActivateWorldScriptObject(g_levelScript);
    }
}

void EngineRenderTick(hFloat tick, Heart::HeartEngine* engine)
{
    g_rndCtx->SetRendererCamera( g_viewCamera );
    g_rndCtx->SetWorldMatrix( Heart::hMatrixFunc::identity() );
    g_rndCtx->ClearTarget( hTrue, Heart::BLACK, hTrue, 1.f );

    SimpleSprite::Draw( g_rndCtx );

    Heart::hdRenderCommandBuffer cmdBuf = g_rndCtx->SaveToCommandBuffer();

    engine->GetRenderer()->SubmitRenderCommandBuffer( cmdBuf, hTrue );
}

bool EngineShutdownRequest(Heart::HeartEngine* engine)
{
    engine->GetEntityFactory()->DeactivateWorldScriptObject();
    HEART_RESOURCE_SAFE_RELEASE(g_levelScript);
    hDELETE_SAFE(hGeneralHeap, g_initPackage);
    hDELETE_SAFE(hGeneralHeap, g_viewCamera);
    engine->GetRenderer()->DestroyRenderSubmissionCtx(g_rndCtx);
    return hTrue;
}

void EngineShutdown(Heart::HeartEngine*)
{

}
