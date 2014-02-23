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
    void hHeartEngine::ProtoBufLogHandler(google::protobuf::LogLevel level, const char* filename, int line, const std::string& message) {
        hcPrintf("ProtoBuf Log: %s, file: %s(%u)", message.c_str(), filename, line);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hHeartEngine::hHeartEngine(const hChar* rootdir, hConsoleOutputProc consoleCb, void* consoleUser, hdDeviceConfig* deviceConfig)
        : firstLoaded_(hNullptr)
        , coreAssetsLoaded_(hNullptr)
        , mainUpdate_(hNullptr)
        , mainRender_(hNullptr)
        , shutdownUpdate_(hNullptr)
        , onShutdown_(hNullptr)
    {
        GOOGLE_PROTOBUF_VERIFY_VERSION;

#ifdef HEART_USE_SDL2
        hUint32 sdlFlags = 0;
        sdlFlags |= SDL_INIT_VIDEO;
        sdlFlags |= SDL_INIT_GAMECONTROLLER;
        sdlFlags |= SDL_INIT_HAPTIC;
        sdlFlags |= SDL_INIT_EVENTS;
        SDL_Init(sdlFlags);
#endif

        hdGetCurrentWorkingDir(workingDir_.GetBuffer(), workingDir_.GetMaxSize());
        hdGetProcessDirectory(processDir_.GetBuffer(), processDir_.GetMaxSize());

        hcPrintf("Current Directory: %s\nProcess Directory: %s", workingDir_.GetBuffer(), processDir_.GetBuffer());

        hdMountPoint(processDir_, "proc");
        hdMountPoint(workingDir_, "cd");
        hdMountPoint("cd:/CONFIG/", "cfg");
        hdMountPoint("cd:/SCRIPT/", "script");
        hdMountPoint("cd:/GAMEDATA/", "data");

        google::protobuf::SetLogHandler(&hHeartEngine::ProtoBufLogHandler);

        if (rootdir) hStrCopy(workingDir_.GetBuffer(), workingDir_.GetMaxSize(), rootdir);
        else hSysCall::GetCurrentWorkingDir(workingDir_.GetBuffer(), workingDir_.GetMaxSize());

        hUint end = hStrLen(workingDir_.GetBuffer())-1;
        if (workingDir_[end] != '\\' && workingDir_[end] != '/') {
            hStrCat(workingDir_.GetBuffer(), workingDir_.GetMaxSize(), "/");
        }

        hNetwork::initialise();

        //////////////////////////////////////////////////////////////////////////
        // Create engine classes /////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        mainPublisherCtx_ = hNEW(hPublisherContext);
        jobManager_ = hNEW(hJobManager);
        actionManager_ = hNEW(hActionManager);
        system_ = hNEW(hSystem);
        fileMananger_ = hNEW(hDriveFileSystem)();
        resourceMananger_ = hNEW(hResourceManager);
        renderer_ = hNEW(hRenderer);
        soundManager_ = hNEW(hSoundManager);
        console_ = hNEW(hSystemConsole)(consoleCb, consoleUser);
        luaVM_ = hNEW(hLuaStateManager);
        debugMenuManager_ = hNEW(hDebugMenuManager);
        debugServer_=hNEW(hNetHost);

        //////////////////////////////////////////////////////////////////////////
        // Read in the configFile_ ///////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        configFile_.readConfig( "cfg:/config.cfg", fileMananger_ );

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
        
        hcSetOutputStringCallback(hSystemConsole::printConsoleMessage);
        debugServer_->initialise(configFile_.getOptionInt("debug.server.port", 8335));
        mainPublisherCtx_->initialise(1024*1024);
        system_->Create(config_, deviceConfig_);
        jobManager_->initialise();
        actionManager_->initialise(system_);
        hClock::initialise();
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
        resourceMananger_->initialise( this, renderer_, fileMananger_, jobManager_, NULL );
        soundManager_->Initialise();
        luaVM_->Initialise();

        g_ProfilerManager_ = hNEW(hProfilerManager)();

        ///////////////////////////////////////////////////////////////////////////////////////////////////
        // Initialise Engine scripting elements ///////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////

        OpenHeartLuaLib(luaVM_->GetMainState(), this);
        renderer_->GetMaterialManager()->openLuaMaterialLib(luaVM_->GetMainState());
        actionManager_->registerLuaLib(luaVM_);

        //Run the start up script
        hIFile* startupscript = fileMananger_->OpenFile("script:/startup.lua", FILEMODE_READ);
        if (startupscript)
        {
            hChar* script=hNullptr;
            if (startupscript->getIsMemMapped()) {
                script=(hChar*)startupscript->getMemoryMappedBase();
            } else {
                script = (hChar*)hAlloca((hSize_t)startupscript->Length()+1);
                startupscript->Read(script, (hUint)startupscript->Length());
            }
            luaL_loadbuffer(luaVM_->GetMainState(), script, startupscript->Length(), "script:/startup.lua");
            if (lua_pcall(luaVM_->GetMainState(), 0, LUA_MULTRET, 0) != 0) {
                hcAssertFailMsg("startup.lua Failed to run, Error: %s", lua_tostring(luaVM_->GetMainState(), -1));
                lua_pop(luaVM_->GetMainState(), 1);
            }
            fileMananger_->CloseFile(startupscript);
        }

        //////////////////////////////////////////////////////////////////////////
        // Do any post start up init /////////////////////////////////////////////
        // Console needs resources, call after setup functions ///////////////////
        //////////////////////////////////////////////////////////////////////////
        renderer_->GetMaterialManager()->createDebugMaterials();
        renderer_->initialiseCameras();
        console_->initialise(actionManager_, luaVM_, resourceMananger_, renderer_, mainPublisherCtx_, debugServer_);
        debugMenuManager_->Initialise(renderer_, resourceMananger_, actionManager_);

        debugInfo_ = hNEW(hDebugInfo)(this);
        debugMenuManager_->RegisterMenu("dbinfo", debugInfo_);
        debugMenuManager_->SetMenuVisiablity("dbinfo", true);

        //////////////////////////////////////////////////////////////////////////
        // Load core assets - are none now... ////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        engineState_ = hHeartState_LoadingCore;

        hMemTracking::TrackPushMarker("After_Engine_Init");
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
        hBool runnableState=engineState_==hHeartState_Running||engineState_==hHeartState_Paused;

        {
            HEART_PROFILE_SCOPE("MainUpdate");

            frameTimer.reset();
            hClock::update();
            GetSystem()->Update();
            debugServer_->service();
            GetResourceManager()->update();
            getActionManager()->update();
            GetConsole()->update();
            if (GetSystem()->exitSignaled()) {
                if (!shutdownUpdate_ || shutdownUpdate_(this)) {
                    //wait on game to say ok to shutdown
                    //GetJobManager()->Destory(); // Should this be here?
                    if (onShutdown_ && runnableState) onShutdown_(this);
                    engineState_ = hHeartState_ShuttingDown;
                    return;
                }
            }
            GetSoundManager()->Update();
            if (mainUpdate_ && runnableState)
                (*mainUpdate_)( this );
            GetVM()->Update();
            //before calling Update dispatch the last frames messages
            GetMainEventPublisher()->updateDispatch();
            debugMenuManager_->PreRenderUpdate();

            /*
             * Begin new frame of draw calls
             **/
            GetRenderer()->BeginRenderFrame();
            debugServer_->printDebugInfo((hFloat)GetRenderer()->GetWidth(), (hFloat)GetRenderer()->GetHeight());

            if (mainRender_ && runnableState)
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
            
            getActionManager()->endOfFrameUpdate();
        }

#ifdef HEART_DO_PROFILE
        g_ProfilerManager_->SetFrameTime(frameTimer.elapsedMicroSec()/1000.f);
#endif
        debugMenuManager_->EndFrameUpdate();
    }

    hHeartEngine::~hHeartEngine()
    {
        hMemTracking::TrackPopMarker();

        debugMenuManager_->Destroy();
        console_->destroy();
        soundManager_->Destory(); 
        resourceMananger_->shutdown( renderer_ );
        luaVM_->Destroy();
        renderer_->Destroy();
        debugServer_->destroy();
        jobManager_->shutdown();

        //hDELETE_SAFE(GetGlobalHeap(), debugInfo_);
        hDELETE_SAFE(g_ProfilerManager_);
        hDELETE_SAFE(debugMenuManager_);
        hDELETE_SAFE(luaVM_);
        hDELETE_SAFE(console_);
        hDELETE_SAFE(resourceMananger_);
        hDELETE_SAFE(soundManager_);
        hDELETE_SAFE(renderer_);
        hDELETE_SAFE(actionManager_);
        hDELETE_SAFE(fileMananger_);
        hDELETE_SAFE(jobManager_);
        hDELETE_SAFE(mainPublisherCtx_);
        hDELETE_SAFE(system_);
        hDELETE_SAFE(debugServer_);

        hNetwork::shutdown();

        // Delete all global objects allocated by libprotobuf.
        google::protobuf::ShutdownProtobufLibrary();
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hHeartEngine::DoEngineTick()
    {
        static int framecount=0;
        if (framecount==1)
        {
            heart_thread_prof_begin("profile_frame.prof");
        }
        switch(engineState_)
        {
        case hHeartState_LoadingCore:
            {
                DoUpdate();
                PostCoreResourceLoad();
                if (coreAssetsLoaded_)
                {
                    (*coreAssetsLoaded_)(this);
                }
                engineState_ = hHeartState_Running;
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
        if (framecount==1)
        {
            heart_thread_prof_end(0);
        }
        framecount=0;
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

    Heart::hHeartEngine* engine = hNEW(Heart::hHeartEngine) (NULL, NULL, NULL, &deviceConfig);

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
    Heart::hSysCall::hInitSystemDebugLibs();
    heart_thread_prof_begin("profile_startup.prof");
    Heart::hdDeviceConfig deviceConfig;
    deviceConfig.instance_ = hInstance;
    deviceConfig.hWnd_ = hWnd;

    Heart::hHeartEngine* engine = 
        hNEW(Heart::hHeartEngine) (callbacks->overrideFileRoot_, callbacks->consoleCallback_, callbacks->consoleCallbackUser_, &deviceConfig);

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

    heart_thread_prof_end(0);
    return engine;
}

#else
#   error ("Platform not supported")
#endif

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

    hDELETE(engine);
}
