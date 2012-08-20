/********************************************************************

filename:     hHeart.cpp    

Copyright (c) 4:2:2012 James Moran

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

/************************************************************************/
/*
Engine Memory Heap allocations
*/
/************************************************************************/
// #define MB                ( 1024 * 1024 )
// 
// Heart::hMemoryHeap hDebugHeap;
// Heart::hMemoryHeap hRendererHeap;
// Heart::hMemoryHeap hResourceHeap;
// Heart::hMemoryHeap hSceneGraphHeap;
// Heart::hMemoryHeap hGeneralHeap;
// Heart::hMemoryHeap hVMHeap;
// Heart::hMemoryHeap hUIHeap;

/************************************************************************/
/*    Engine Init                                                         */
/************************************************************************/

namespace Heart
{

    hChar           HeartEngine::HEART_VERSION_STRING[] = "HeartEngine v0.4";
    const hFloat    HeartEngine::HEART_VERSION = 0.4f;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HeartEngine::HeartEngine( const hChar* configFile, hdDeviceConfig* deviceConfig )
    {
        //
        // Create engine classes
        //
        eventManager_ = hNEW(GetGlobalHeap(), EventManager);

        jobManager_ = hNEW_ALIGN(GetGlobalHeap(), 32, hJobManager);

        controllerManager_ = hNEW(GetGlobalHeap() , hControllerManager);

        system_ = hNEW(GetGlobalHeap(), hSystem);

        fileMananger_ = hNEW(GetGlobalHeap(), hDriveFileSystem);

        resourceMananger_ = hNEW(GetGlobalHeap(), hResourceManager);

        renderer_ = hNEW(GetGlobalHeap(), hRenderer);

        soundManager_ = hNEW(GetGlobalHeap(), hSoundManager);

        console_ = hNEW(GetGlobalHeap(), hSystemConsole);

        uiRenderer_ = hNEW(GetGlobalHeap(), hGwenRenderer);

        luaVM_ = hNEW(GetGlobalHeap(), hLuaStateManager);

        entityFactory_ = hNEW(GetGlobalHeap(), hEntityFactory);

        debugMenuManager_ = hNEW(GetDebugHeap(), hDebugMenuManager);

        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        rtProfileMenu_ = NULL;

        //////////////////////////////////////////////////////////////////////////
        // Read in the configFile_ ////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        if ( configFile )
            configFile_.ReadConfig( configFile, fileMananger_ );
        else
            configFile_.ReadConfig( "config.cfg", fileMananger_ );

        config_.Width_ = configFile_.GetScreenWidth();
        config_.Height_ = configFile_.GetScreenHeight();
        config_.Fullscreen_ = configFile_.GetFullscreen();
        config_.vsync_ = configFile_.GetVsync();

        deviceConfig_ = *deviceConfig;
        deviceConfig_.width_ = config_.Width_;
        deviceConfig_.height_ = config_.Height_;

        //////////////////////////////////////////////////////////////////////////
        // Register Engine Level Event Channels //////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        // Init Engine Classes ///////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        
        hcSetOutputStringCallback(hSystemConsole::PrintConsoleMessage);

        system_->Create( config_, deviceConfig_ );

        jobManager_->Initialise();

        controllerManager_->Initialise(system_);

        hClock::Initialise();

        renderer_->Create( 
            system_,
            config_.Width_,
            config_.Height_,
            config_.bpp_,
            config_.MinShaderVersion_,
            config_.Fullscreen_,
            config_.vsync_,
            resourceMananger_
            );

        resourceMananger_->Initialise( this, renderer_, fileMananger_, NULL );

        soundManager_->Initialise();

        hIFileSystem* luaFilesystems[] = 
        {
            fileMananger_,
            NULL
        };

        luaVM_->Initialise( luaFilesystems );

        entityFactory_->Initialise( fileMananger_, resourceMananger_, this );

        //////////////////////////////////////////////////////////////////////////
        // Console needs resources, call after setup functions ///////////////////
        //////////////////////////////////////////////////////////////////////////

        console_->Initialise( controllerManager_, luaVM_, resourceMananger_, renderer_, uiRenderer_ );

        uiRenderer_->Initialise(renderer_, resourceMananger_);

        debugMenuManager_->Initialise(uiRenderer_, renderer_, resourceMananger_, controllerManager_);

#if defined HEART_DO_PROFILE
        g_ProfilerManager_ = hNEW(GetDebugHeap(), hProfilerManager)();
#endif

        ///////////////////////////////////////////////////////////////////////////////////////////////////
        // Initialise Engine scripting elements ///////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////

        RegisterDefaultComponents();

        OpenHeartLuaLib(luaVM_->GetMainState(), this);

        engineState_ = hHeartState_LoadingCore;

        GetResourceManager()->mtLoadPackage("CORE");

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void HeartEngine::RegisterDefaultComponents()
    {
        HEART_REGISTER_COMPONENT_FACTORY( entityFactory_, hLuaScriptComponent, 
            ComponentCreateCallback::bind< hLuaScriptComponent::hLuaComponentCreate >(),
            ComponentDestroyCallback::bind< hLuaScriptComponent::hLuaComponentDestroy >() );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void HeartEngine::DoUpdate()
    {
#ifdef HEART_DO_PROFILE
        g_ProfilerManager_->BeginFrame();
#endif
        hTimer frameTimer;

        {
            HEART_PROFILE_SCOPE("MainUpdate");

            hClock::BeginTimer(frameTimer);
            hClock::Update();

            GetSystem()->Update();

            GetResourceManager()->MainThreadUpdate();

            GetControllerManager()->Update();

            //before calling Update dispatch the last frames messages
            GetEventManager()->DispatchEvents();

            GetConsole()->Update();

    //         if ( (*quitFlag) )
    //         {
    //             if ( config->shutdownTickFunc_( pEngine ) )
    //             {
    //                 //wait on game to say ok to shutdown
    //                 GetJobManager()->Destory();
    //                 break;
    //             }
    //         }

            GetSoundManager()->Update();

            (*mainUpdate_)( this );

            GetVM()->Update();

            GetRenderer()->BeginRenderFrame();

            (*mainRender_)( this );

            debugMenuManager_->RenderMenus();

            GetRenderer()->EndRenderFrame();

            GetControllerManager()->EndOfFrameUpdate();

            hClock::EndTimer(frameTimer);
        }

        if (rtProfileMenu_)
        {
            rtProfileMenu_->EndFrameUpdate(frameTimer.ElapsedmS()/1000.f, GetRenderer()->GetLastGPUTime());
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void HeartEngine::DoUpdateNoApp()
    {
#ifdef HEART_DO_PROFILE
        g_ProfilerManager_->BeginFrame();
#endif

        {
            HEART_PROFILE_SCOPE("MainUpdateNoApp");

            hClock::Update();

            GetSystem()->Update();

            GetResourceManager()->MainThreadUpdate();

            GetControllerManager()->Update();

            //before calling Update dispatch the last frames messages
            GetEventManager()->DispatchEvents();

            GetConsole()->Update();

    //         if ( (*quitFlag) )
    //         {
    //             GetJobManager()->Destory();
    //             break;
    //         }

            GetSoundManager()->Update();

            GetVM()->Update();

            GetRenderer()->BeginRenderFrame();

            debugMenuManager_->RenderMenus();

            GetRenderer()->EndRenderFrame();

            GetControllerManager()->EndOfFrameUpdate();
        }
    }


    HeartEngine::~HeartEngine()
    {
        //eventManager_->RemoveChannel( KERNEL_EVENT_CHANNEL );

        jobManager_->Destory();

        debugMenuManager_->Destroy();

        uiRenderer_->DestroyResources();

        console_->Destroy();

        soundManager_->Destory(); 

        resourceMananger_->Shutdown( renderer_ );

        luaVM_->Destroy();

        renderer_->Destroy();

        hDELETE_SAFE(GetDebugHeap(), debugMenuManager_);

        hDELETE_SAFE(GetGlobalHeap(), entityFactory_);

        hDELETE(GetGlobalHeap(), luaVM_);

        hDELETE_SAFE(GetGlobalHeap(), uiRenderer_);

        hDELETE(GetGlobalHeap(), console_);

        hDELETE(GetGlobalHeap(), soundManager_);

        hDELETE(GetGlobalHeap(), renderer_);

        hDELETE(GetGlobalHeap(), controllerManager_);

        hDELETE(GetGlobalHeap(), resourceMananger_);

        hDELETE(GetGlobalHeap(), fileMananger_);

        hDELETE_ALIGNED(GetGlobalHeap(), jobManager_);

        hDELETE(GetGlobalHeap(), eventManager_);

        hDELETE(GetGlobalHeap(), system_);

    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void HeartEngine::DoEngineTick()
    {
        switch(engineState_)
        {
        case hHeartState_LoadingCore:
            {
                DoUpdateNoApp();

                if (GetResourceManager()->RequiredResourcesReady())
                {
                    PostCoreResourceLoad();
                    if (sharedLib_ != HEART_SHAREDLIB_INVALIDADDRESS)
                    {
                        (*coreAssetsLoaded_)(this);
                    }
                    engineState_ = hHeartState_Running;
                }
            }
            break;
        case hHeartState_Running:
            {
                if (sharedLib_ == HEART_SHAREDLIB_INVALIDADDRESS)
                {
                    DoUpdateNoApp();
                }
                else
                {
                    DoUpdate();
                }
            }
            break;
        default:
            //TODO
            break;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void HeartEngine::PostCoreResourceLoad()
    {
        //////////////////////////////////////////////////////////////////////////
        // Create debug menus ////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        rtProfileMenu_ = hNEW(GetDebugHeap(), hRTProfilerMenu)(debugMenuManager_->GetDebugCanvas());
        debugMenuManager_->RegisterMenu("rtp", rtProfileMenu_);
    }

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
    HEARTCORE_SLIBEXPORT Heart::HeartEngine* HEART_API hHeartInitEngine( const hChar* appLib, HINSTANCE hInstance )
#else
    #error ("Platform not supported")
#endif
{
    static hByte g_globalMemoryPoolSpace[sizeof(Heart::hMemoryHeap)];
    static hByte g_debugMemoryPoolSpace[sizeof(Heart::hMemoryHeap)];
    Heart::hdDeviceConfig deviceConfig;

#ifdef _WIN32
    deviceConfig.instance_ = hInstance;
#else
    #error ("Platform not supported")
#endif

    Heart::SetGlobalHeap(hPLACEMENT_NEW(g_globalMemoryPoolSpace) Heart::hMemoryHeap);
    Heart::SetDebugHeap(hPLACEMENT_NEW(g_debugMemoryPoolSpace) Heart::hMemoryHeap);

    Heart::GetGlobalHeap()->create(1024*1024,hFalse);
    Heart::GetDebugHeap()->create(1024*1024,hFalse);

    Heart::HeartEngine* engine = hNEW(Heart::GetGlobalHeap(), Heart::HeartEngine) (NULL, &deviceConfig);

    engine->sharedLib_ = Heart::hd_OpenSharedLib(appLib);

    if ( engine->sharedLib_ != HEART_SHAREDLIB_INVALIDADDRESS )
    {
        engine->firstLoaded_        = (Heart::FirstLoaded)Heart::hd_GetFunctionAddress(engine->sharedLib_,      "HeartAppFirstLoaded"      );
        engine->coreAssetsLoaded_   = (Heart::CoreAssetsLoaded)Heart::hd_GetFunctionAddress(engine->sharedLib_, "HeartAppCoreAssetsLoaded" );
        engine->mainUpdate_         = (Heart::MainUpdate)Heart::hd_GetFunctionAddress(engine->sharedLib_,       "HeartAppMainUpate"        );
        engine->mainRender_         = (Heart::MainRender)Heart::hd_GetFunctionAddress(engine->sharedLib_,       "HeartAppMainRender"       );
        engine->shutdownUpdate_     = (Heart::ShutdownUpdate)Heart::hd_GetFunctionAddress(engine->sharedLib_,   "HeartAppShutdownUpdate"   );
        engine->onShutdown_         = (Heart::OnShutdown)Heart::hd_GetFunctionAddress(engine->sharedLib_,       "HeartAppOnShutdown"       );

        if ( 
            !engine->firstLoaded_        ||
            !engine->coreAssetsLoaded_   ||
            !engine->mainUpdate_         ||
            !engine->mainRender_         ||
            !engine->shutdownUpdate_     ||
            !engine->onShutdown_        
            )
        {
            engine->sharedLib_ = HEART_SHAREDLIB_INVALIDADDRESS;
        }
    }

    if (engine->firstLoaded_)
    {
        (*engine->firstLoaded_)(engine);
    }

    return engine;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

HEARTCORE_SLIBEXPORT 
hUint32 HEART_API hHeartDoMainUpdate( Heart::HeartEngine* engine )
{
    if (engine)
    {
        engine->DoEngineTick();
    }

    return (hUint32)engine->GetState();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

HEARTCORE_SLIBEXPORT 
void HEART_API hHeartShutdownEngine( Heart::HeartEngine* engine )
{
    Heart::hd_CloseSharedLib(engine->sharedLib_);

    hDELETE(Heart::GetGlobalHeap(), engine);

    // Check memory usage
#define CHECK_HEAP( x ) x.printLeaks(#x)//hcAssertMsg( x.bytesAllocated() == 0, "Heap "#x" leaking %u bytes", x.bytesAllocated() );

    //         CHECK_HEAP( hGeneralHeap );
    //         CHECK_HEAP( hRendererHeap );
    //         CHECK_HEAP( hResourceHeap );
    //         CHECK_HEAP( hSceneGraphHeap );
    //         CHECK_HEAP( hVMHeap );
    //         CHECK_HEAP(hUIHeap);

#undef CHECK_HEAP
}