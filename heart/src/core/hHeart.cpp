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
/*    Engine Init                                                         */
/************************************************************************/

namespace Heart
{

    extern hProfilerManager* g_ProfilerManager_;

    __declspec(selectany)
    hChar           hHeartEngine::HEART_VERSION_STRING[] = "HeartEngine v0.4";
    __declspec(selectany)
    const hFloat    hHeartEngine::HEART_VERSION = 0.4f;


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hHeartEngine::hHeartEngine(const hChar* rootdir, hConsoleOutputProc consoleCb, void* consoleUser, hdDeviceConfig* deviceConfig)
    {
        if (rootdir) hStrCopy(workingDir_.GetBuffer(), workingDir_.GetMaxSize(), rootdir);
        else hSysCall::GetCurrentWorkingDir(workingDir_.GetBuffer(), workingDir_.GetMaxSize());

        hUint end = hStrLen(workingDir_.GetBuffer())-1;
        if (workingDir_[end] != '\\' && workingDir_[end] != '/') {
            hStrCat(workingDir_.GetBuffer(), workingDir_.GetMaxSize(), "/");
        }

        //////////////////////////////////////////////////////////////////////////
        // Create engine classes /////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        mainPublisherCtx_ = hNEW(GetGlobalHeap(), hPublisherContext);
        jobManager_ = hNEW_ALIGN(GetGlobalHeap(), 32, hJobManager);
        controllerManager_ = hNEW(GetGlobalHeap() , hControllerManager);
        system_ = hNEW(GetGlobalHeap(), hSystem);
        fileMananger_ = hNEW(GetGlobalHeap(), hDriveFileSystem)(workingDir_);
        resourceMananger_ = hNEW(GetGlobalHeap(), hResourceManager);
        renderer_ = hNEW(GetGlobalHeap(), hRenderer);
        soundManager_ = hNEW(GetGlobalHeap(), hSoundManager);
        console_ = hNEW(GetGlobalHeap(), hSystemConsole)(consoleCb, consoleUser);
        luaVM_ = hNEW(GetGlobalHeap(), hLuaStateManager);
        entityFactory_ = hNEW(GetGlobalHeap(), hEntityFactory);
        debugMenuManager_ = hNEW(GetDebugHeap(), hDebugMenuManager);

        //////////////////////////////////////////////////////////////////////////
        // Read in the configFile_ ///////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        configFile_.readConfig( "CONFIG/config.cfg", fileMananger_ );

        config_.Width_ = configFile_.getOptionUint("renderer.width", 640);
        config_.Height_ = configFile_.getOptionUint("renderer.height", 480);
        config_.Fullscreen_ = configFile_.getOptionBool("renderer.fullscreen", false);
        config_.vsync_ = configFile_.getOptionBool("renderer.vsync", false);

        deviceConfig_ = *deviceConfig;
        deviceConfig_.width_ = config_.Width_;
        deviceConfig_.height_ = config_.Height_;

        //////////////////////////////////////////////////////////////////////////
        // Init Engine Classes ///////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        
        hcSetOutputStringCallback(hSystemConsole::PrintConsoleMessage);
        mainPublisherCtx_->initialise(GetGlobalHeap(), 1024*1024);
        system_->Create(config_, deviceConfig_);
        jobManager_->Initialise();
        controllerManager_->Initialise(system_);
        hClock::Initialise();
        config_.Width_ = system_->getWindowWidth();
        config_.Height_ = system_->getWindowHeight();
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
        luaVM_->Initialise();
        entityFactory_->initialise( fileMananger_, resourceMananger_, this );

        //////////////////////////////////////////////////////////////////////////
        // Console needs resources, call after setup functions ///////////////////
        //////////////////////////////////////////////////////////////////////////
        console_->Initialise(controllerManager_, luaVM_, resourceMananger_, renderer_, mainPublisherCtx_);
        debugMenuManager_->Initialise(renderer_, resourceMananger_, controllerManager_);
        g_ProfilerManager_ = hNEW(GetDebugHeap(), hProfilerManager)();

        ///////////////////////////////////////////////////////////////////////////////////////////////////
        // Initialise Engine scripting elements ///////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////

        RegisterDefaultComponents();

        OpenHeartLuaLib(luaVM_->GetMainState(), this);
        renderer_->GetMaterialManager()->openLuaMaterialLib(luaVM_->GetMainState());

        //Run the start up script
        hIFile* startupscript = fileMananger_->OpenFileRoot("SCRIPT/startup.lua", FILEMODE_READ);
        if (startupscript)
        {
            hChar* script = (hChar*)hAlloca((hSizeT)startupscript->Length()+1);
            startupscript->Read(script, (hUint)startupscript->Length());
            script[startupscript->Length()] = 0;
            if (luaL_dostring(luaVM_->GetMainState(), script) != 0) {
                hcAssertFailMsg("startup.lua Failed to run, Error: %s", lua_tostring(luaVM_->GetMainState(), -1));
                lua_pop(luaVM_->GetMainState(), 1);
            }
            fileMananger_->CloseFile(startupscript);
        }

        //////////////////////////////////////////////////////////////////////////
        // Do any post start up init /////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        renderer_->GetMaterialManager()->createDebugMaterials();
        renderer_->initialiseCameras();

        //////////////////////////////////////////////////////////////////////////
        // Load core assets - are none now... ////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        engineState_ = hHeartState_LoadingCore;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hHeartEngine::RegisterDefaultComponents()
    {
        HEART_REGISTER_COMPONENT_FACTORY( entityFactory_, hLuaScriptComponent, 
            ComponentCreateCallback::bind< hLuaScriptComponent::hLuaComponentCreate >(),
            ComponentDestroyCallback::bind< hLuaScriptComponent::hLuaComponentDestroy >() );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hHeartEngine::DoUpdate()
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
            GetConsole()->Update();
            if (GetSystem()->ExitSignaled()) {
                if (!shutdownUpdate_ || shutdownUpdate_(this)) {
                    //wait on game to say ok to shutdown
                    //GetJobManager()->Destory(); // Should this be here?
                    if (onShutdown_) onShutdown_(this);
                    engineState_ = hHeartState_ShuttingDown;
                    return;
                }
            }
            GetSoundManager()->Update();
            if (mainUpdate_)
                (*mainUpdate_)( this );
            GetVM()->Update();
            //before calling Update dispatch the last frames messages
            GetMainEventPublisher()->updateDispatch();
            debugMenuManager_->PreRenderUpdate();

            /*
             * Begin new frame of draw calls
             **/
            GetRenderer()->BeginRenderFrame();
            if (mainRender_)
                (*mainRender_)( this );

            GetRenderer()->rendererFrameSubmit();
            /*
             * Frame isn't swapped until next call to EndRenderFrame() so
             * can render to back buffer here
             **/
            debugMenuManager_->RenderMenus(
                GetRenderer()->GetMainSubmissionCtx(), 
                GetRenderer()->GetMaterialManager());
            /*
             * Swap back buffer and Submit to GPU draw calls sent to renderer in mainRender()
             **/
            GetRenderer()->EndRenderFrame();
            
            GetControllerManager()->EndOfFrameUpdate();
            hClock::EndTimer(frameTimer);
        }

#ifdef HEART_DO_PROFILE
        g_ProfilerManager_->SetFrameTime(frameTimer.ElapsedmS()/1000.f);
#endif
        debugMenuManager_->EndFrameUpdate();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hHeartEngine::DoUpdateNoApp()
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
            GetConsole()->Update();
            if (GetSystem()->ExitSignaled()) {
                exit(0);// emergency exit
            }
            GetSoundManager()->Update();
            GetVM()->Update();
            //before calling Update dispatch the last frames messages
            GetMainEventPublisher()->updateDispatch();

            debugMenuManager_->PreRenderUpdate();
            /*
             * Begin new frame of draw calls
             **/
            GetRenderer()->BeginRenderFrame();
            GetRenderer()->rendererFrameSubmit();
            /*
             * Frame isn't swapped until next call to EndRenderFrame() so
             * can render to back buffer here
             **/
            debugMenuManager_->RenderMenus(
                GetRenderer()->GetMainSubmissionCtx(), 
                GetRenderer()->GetMaterialManager());
            /*
             * Swap back buffer and Submit to GPU draw calls sent to renderer in mainRender()
             **/
            GetRenderer()->EndRenderFrame();

            GetControllerManager()->EndOfFrameUpdate();
        }

        debugMenuManager_->EndFrameUpdate();
    }


    hHeartEngine::~hHeartEngine()
    {
        jobManager_->Destory();
        debugMenuManager_->Destroy();
        console_->Destroy();
        soundManager_->Destory(); 
        resourceMananger_->Shutdown( renderer_ );
        luaVM_->Destroy();
        renderer_->Destroy();

        hDELETE_SAFE(GetDebugHeap(), g_ProfilerManager_);
        hDELETE_SAFE(GetDebugHeap(), debugMenuManager_);
        hDELETE_SAFE(GetGlobalHeap(), entityFactory_);
        hDELETE_SAFE(GetGlobalHeap(), luaVM_);
        hDELETE_SAFE(GetGlobalHeap(), console_);
        hDELETE_SAFE(GetGlobalHeap(), resourceMananger_);
        hDELETE_SAFE(GetGlobalHeap(), soundManager_);
        hDELETE_SAFE(GetGlobalHeap(), renderer_);
        hDELETE_SAFE(GetGlobalHeap(), controllerManager_);
        hDELETE_SAFE(GetGlobalHeap(), fileMananger_);
        hDELETE_ALIGNED(GetGlobalHeap(), jobManager_);
        hDELETE_SAFE(GetGlobalHeap(), mainPublisherCtx_);
        hDELETE_SAFE(GetGlobalHeap(), system_);

    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hHeartEngine::DoEngineTick()
    {
        switch(engineState_)
        {
        case hHeartState_LoadingCore:
            {
                DoUpdateNoApp();

                if (GetResourceManager()->RequiredResourcesReady())
                {
                    PostCoreResourceLoad();
                    if (coreAssetsLoaded_)
                    {
                        (*coreAssetsLoaded_)(this);
                    }
                    engineState_ = hHeartState_Running;
                }
            }
            break;
        case hHeartState_Running: 
        case hHeartState_Paused: {
                DoUpdate();
            }
            break;
        case hHeartState_ShuttingDown:
            engineState_ = hHeartState_Finised;
            break;
        case hHeartState_Finised:
            break;
        default:
            hcAssertFailMsg("Engine in unknown state!");
            break;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hHeartEngine::PostCoreResourceLoad()
    {
        //////////////////////////////////////////////////////////////////////////
        // Create debug menus ////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
#ifdef HEART_DEBUG
#pragma message ("TODO- Real time profiler menu")
/*        hRTProfilerMenu* rtProfileMenu_ = hNEW(GetDebugHeap(), hRTProfilerMenu)(debugMenuManager_->GetDebugCanvas(), GetRenderer());
        rtProfileMenu_->SetHidden(hTrue);
        debugMenuManager_->RegisterMenu("rtp", rtProfileMenu_);

        hMemoryViewMenu* mvMenu_ = hNEW(GetDebugHeap(), hMemoryViewMenu)(debugMenuManager_->GetDebugCanvas());
        //mvMenu_->SetHidden(hTrue);
        debugMenuManager_->RegisterMenu("mv", mvMenu_);*/
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hChar* hHeartEngine::VersionString()
    {
        return HEART_VERSION_STRING;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hFloat hHeartEngine::Version()
    {
        return HEART_VERSION;
    }

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
    HEART_DLLEXPORT Heart::hHeartEngine* HEART_API hHeartInitEngineFromSharedLib(const hChar* appLib, HINSTANCE hInstance, HWND hWnd)

{
    Heart::hdDeviceConfig deviceConfig;
    deviceConfig.instance_ = hInstance;
    deviceConfig.hWnd_ = hWnd;

    Heart::hHeartEngine* engine = hNEW(Heart::GetGlobalHeap(), Heart::hHeartEngine) (NULL, NULL, NULL, &deviceConfig);

    engine->sharedLib_ = Heart::hd_OpenSharedLib(appLib);

    if ( engine->sharedLib_ != HEART_SHAREDLIB_INVALIDADDRESS )
    {
        engine->firstLoaded_        = (hFirstLoadedProc)Heart::hd_GetFunctionAddress(engine->sharedLib_,      "HeartAppFirstLoaded"      );
        engine->coreAssetsLoaded_   = (hCoreAssetsLoadedProc)Heart::hd_GetFunctionAddress(engine->sharedLib_, "HeartAppCoreAssetsLoaded" );
        engine->mainUpdate_         = (hMainUpdateProc)Heart::hd_GetFunctionAddress(engine->sharedLib_,       "HeartAppMainUpate"        );
        engine->mainRender_         = (hMainRenderProc)Heart::hd_GetFunctionAddress(engine->sharedLib_,       "HeartAppMainRender"       );
        engine->shutdownUpdate_     = (hShutdownUpdateProc)Heart::hd_GetFunctionAddress(engine->sharedLib_,   "HeartAppShutdownUpdate"   );
        engine->onShutdown_         = (hOnShutdownProc)Heart::hd_GetFunctionAddress(engine->sharedLib_,       "HeartAppOnShutdown"       );

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

HEART_DLLEXPORT Heart::hHeartEngine* HEART_API hHeartInitEngine(hHeartEngineCallbacks* callbacks, HINSTANCE hInstance, HWND hWnd)
{
    Heart::hdDeviceConfig deviceConfig;
    deviceConfig.instance_ = hInstance;
    deviceConfig.hWnd_ = hWnd;

    Heart::hHeartEngine* engine = 
        hNEW(Heart::GetGlobalHeap(), Heart::hHeartEngine) (callbacks->overrideFileRoot_, callbacks->consoleCallback_, callbacks->consoleCallbackUser_, &deviceConfig);

    engine->firstLoaded_        = callbacks->firstLoaded_;
    engine->coreAssetsLoaded_   = callbacks->coreAssetsLoaded_;
    engine->mainUpdate_         = callbacks->mainUpdate_;
    engine->mainRender_         = callbacks->mainRender_;
    engine->shutdownUpdate_     = callbacks->shutdownUpdate_;
    engine->onShutdown_         = callbacks->onShutdown_;

    if (engine->firstLoaded_)
    {
        (*engine->firstLoaded_)(engine);
    }

    return engine;
}

#else
#   error ("Platform not supported")
#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void initialiseBaseMemoryHeaps()
{
    static hByte g_globalMemoryPoolSpace[sizeof(Heart::hMemoryHeap)];
    static hByte g_debugMemoryPoolSpace[sizeof(Heart::hMemoryHeap)];
    static hByte g_luaMemoryPoolSpace[sizeof(Heart::hMemoryHeap)];

#ifdef HEART_TRACK_MEMORY_ALLOCS
    Heart::hMemTracking::InitMemTracking();
#endif

    Heart::SetGlobalHeap(hPLACEMENT_NEW(g_globalMemoryPoolSpace) Heart::hMemoryHeap("GlobalHeap"));
    Heart::SetDebugHeap(hPLACEMENT_NEW(g_debugMemoryPoolSpace) Heart::hMemoryHeap("DebugHeap"));
    Heart::SetLuaHeap(hPLACEMENT_NEW(g_luaMemoryPoolSpace) Heart::hMemoryHeap("LuaHeap"));

    // It important that the debug heap is created first
    Heart::GetDebugHeap()->create(1024*1024,hFalse);
    Heart::GetGlobalHeap()->create(1024*1024,hFalse);
    Heart::GetLuaHeap()->create(1024*1024,hTrue);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

HEART_DLLEXPORT 
hUint32 HEART_API hHeartDoMainUpdate( Heart::hHeartEngine* engine )
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

HEART_DLLEXPORT 
void HEART_API hHeartShutdownEngine( Heart::hHeartEngine* engine )
{
    Heart::hd_CloseSharedLib(engine->sharedLib_);

    hDELETE(Heart::GetGlobalHeap(), engine);
}
