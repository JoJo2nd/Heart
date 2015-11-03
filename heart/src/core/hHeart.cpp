/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "Heart.h"
#include "lua/hLuaHeartLib.h"
#include "render/hImGuiRenderer.h"
#include "memtracker.h"

namespace Heart {
    hChar           hHeartEngine::HEART_VERSION_STRING[] = "HeartEngine v0.4";
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

    hHeartEngine::hHeartEngine(const hChar* config_script, hSize_t config_script_len)
        : firstLoaded_(nullptr)
        , coreAssetsLoaded_(nullptr)
        , mainUpdate_(nullptr)
        , mainRender_(nullptr)
        , shutdownUpdate_(nullptr)
        , onShutdown_(nullptr) {
        GOOGLE_PROTOBUF_VERIFY_VERSION;

        //////////////////////////////////////////////////////////////////////////
        // Read in the config ////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        luaVM_ = new hLuaStateManager;
        luaVM_->Initialise();
        hConfigurationVariables::loadCVars(luaVM_->GetMainState(), config_script, config_script_len);

        hUint32 sdlFlags = 0;
        sdlFlags |= SDL_INIT_VIDEO;
        sdlFlags |= SDL_INIT_GAMECONTROLLER;
        sdlFlags |= SDL_INIT_HAPTIC;
        sdlFlags |= SDL_INIT_EVENTS;
        SDL_Init(sdlFlags);

        hInitFreetype2();

        hRenderer::loadRendererModule(hConfigurationVariables::getCVarStr("plugin.renderer", "none"), &cvars);

        hFileSystemInterface fileInterface;
        auto plugin_loaded = loadFileSystemInterface(hConfigurationVariables::getCVarStr("plugin.filesystem", "none"), &fileInterface);
        hcAssertMsg(plugin_loaded, "Failed to load file interface \"%s\"", hConfigurationVariables::getCVarStr("plugin.filesystem", "none"));
        fileMananger_ = new hIFileSystem(fileInterface);

        fileMananger_->getCurrentWorkingDir(workingDir_.GetBuffer(), workingDir_.GetMaxSize());
        fileMananger_->getProcessDirectory(processDir_.GetBuffer(), processDir_.GetMaxSize());

        hcPrintf("Current Directory: %s\nProcess Directory: %s", workingDir_.GetBuffer(), processDir_.GetBuffer());

        // current working is already mounted to /
        fileMananger_->mountPoint(processDir_, "/proc");
        fileMananger_->mountPoint("/scripts", "/script");
        fileMananger_->mountPoint("/data", "/data");
        fileMananger_->mountPoint("/proc/tmp", "/tmp");
        fileMananger_->mountPoint("/save", "/save");

        const hChar* script_name = hConfigurationVariables::getCVarStr("startup.script", nullptr);
        hcAssertMsg(script_name, "startup.script is not set");
        hFileHandle start_script_file;
        auto op = fileMananger_->openFile(script_name, FILEMODE_READ, &start_script_file);
        auto er = fileMananger_->fileOpWait(op);
        fileMananger_->fileOpClose(op);
        hcAssertMsg(er == FileError::Ok, "failed to open startup.script \"%s\"", script_name);
        hFileStat scriptfstat;
        op = fileMananger_->fstatAsync(start_script_file, &scriptfstat);
        fileMananger_->fileOpWait(op);
        fileMananger_->fileOpClose(op);

        std::unique_ptr<hChar[]> scriptdata(new hChar[scriptfstat.filesize]);
        op = fileMananger_->freadAsync(start_script_file, scriptdata.get(), scriptfstat.filesize, 0);

        google::protobuf::SetLogHandler(&hHeartEngine::ProtoBufLogHandler);

        /**
        if (rootdir) hStrCopy(workingDir_.GetBuffer(), workingDir_.GetMaxSize(), rootdir);
        else hSysCall::GetCurrentWorkingDir(workingDir_.GetBuffer(), workingDir_.GetMaxSize());

        hUint end = hStrLen(workingDir_.GetBuffer())-1;
        if (workingDir_[end] != '\\' && workingDir_[end] != '/') {
            hStrCat(workingDir_.GetBuffer(), workingDir_.GetMaxSize(), "/");
        }
        */

        hNetwork::initialise();

        mainPublisherCtx_ = new hPublisherContext;
        jobManager = new hJobManager;
        actionManager_ = new hActionManager;
        system_ = new hSystem;
        soundManager_ = nullptr;//new hSoundManager;
        console_ = nullptr;//new hSystemConsole(consoleCb, consoleUser);
        debugMenuManager_ = nullptr;//new hDebugMenuManager;
        debugServer_=new hNetHost;

        config_.Width_ = hConfigurationVariables::getCVarUint("renderer.width", 640);
        config_.Height_ = hConfigurationVariables::getCVarUint("renderer.height", 480);
        config_.Fullscreen_ = hConfigurationVariables::getCVarBool("renderer.fullscreen", false);
        config_.vsync_ = hConfigurationVariables::getCVarBool("renderer.vsync", false);

        //deviceConfig_ = deviceConfig;
        deviceConfig_ = nullptr;

        //////////////////////////////////////////////////////////////////////////
        // Init Engine Classes ///////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        
        //hcSetOutputStringCallback(hSystemConsole::printConsoleMessage);
        debugServer_->initialise(hConfigurationVariables::getCVarInt("debug.server.port", 8335));
        mainPublisherCtx_->initialise(1024*1024);
        system_->Create(config_);
        jobManager->initialise();
        actionManager_->initialise(system_);
        hClock::initialise();
        config_.Width_ = system_->getWindowWidth();
        config_.Height_ = system_->getWindowHeight();
        hRenderer::create( 
            system_,
            config_.Width_,
            config_.Height_,
            config_.bpp_,
            config_.MinShaderVersion_,
            config_.Fullscreen_,
            config_.vsync_);
        hResourceManager::initialise(fileMananger_, jobManager);
        //!!JM todo: soundManager_->Initialise();

        ///////////////////////////////////////////////////////////////////////////////////////////////////
        // Initialise Engine scripting elements ///////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////

        OpenHeartLuaLib(luaVM_->GetMainState(), this);
        actionManager_->registerLuaLib(luaVM_);

        //Run the start up script
        er = fileMananger_->fileOpWait(op);
        hcAssertMsg(er == FileError::Ok, "Failed to read startup script \"%s\"", script_name);
        fileMananger_->fileOpClose(op);
        {
            luaL_loadbuffer(luaVM_->GetMainState(), scriptdata.get(), scriptfstat.filesize, script_name);
            if (lua_pcall(luaVM_->GetMainState(), 0, LUA_MULTRET, 0) != 0) {
                hcAssertFailMsg("startup.lua Failed to run, Error: %s", lua_tostring(luaVM_->GetMainState(), -1));
                lua_pop(luaVM_->GetMainState(), 1);
            }
            fileMananger_->closeFile(start_script_file);
        }

        //////////////////////////////////////////////////////////////////////////
        // Do any post start up init /////////////////////////////////////////////
        // Console needs resources, call after setup functions ///////////////////
        //////////////////////////////////////////////////////////////////////////
        //console_->initialise(actionManager_, luaVM_, renderer_, mainPublisherCtx_, debugServer_); !!JM
        //debugMenuManager_->Initialise(renderer_, actionManager_); !!JM

        debugInfo_ = nullptr;//new hDebugInfo(this);
        //debugMenuManager_->RegisterMenu("dbinfo", debugInfo_); !!JM
        //debugMenuManager_->SetMenuVisiablity("dbinfo", true); !!JM

        //////////////////////////////////////////////////////////////////////////
        // Load core assets - are none now... ////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        // Workaround for dead stripping of these types
        hBool register_type = hTrue;
        register_type &= hShaderProgram::auto_object_registered;
        register_type &= hMaterial::auto_object_registered;
        register_type &= hTTFFontFace::auto_object_registered;
        register_type &= hTextureResource::auto_object_registered;
        register_type &= hEntityDef::auto_object_registered;
        register_type &= hLevel::auto_object_registered;

        if (register_type) {
            engineState_ = hHeartState_LoadingCore;
        }

        mem_track_marker("AfterEngineInit");
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hHeartEngine::DoUpdate() {
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
            hResourceManager::update();
            getActionManager()->update();
            //GetConsole()->update(); !!JM
            if (GetSystem()->exitSignaled()) {
                /*if (!shutdownUpdate_ || shutdownUpdate_(this))*/ {
                    //wait on game to say ok to shutdown
                    //GetJobManager()->Destory(); // Should this be here?
                    // if (onShutdown_ && runnableState) onShutdown_(this); !!JM 
                    engineState_ = hHeartState_ShuttingDown;
                    _exit(0);
                    return;
                }
            }
            //!!JM todo: GetSoundManager()->Update();
            // !!JM culled client side work
            if (mainUpdate_ && runnableState)
                (*mainUpdate_)( this );
            GetVM()->Update();
            //before calling Update dispatch the last frames messages
            GetMainEventPublisher()->updateDispatch();
            //debugMenuManager_->PreRenderUpdate(); !!JM

            /*
             * Begin new frame of draw calls
             **/
            // GetRenderer()->BeginRenderFrame(); !! JM
            // debugServer_->printDebugInfo((hFloat)GetRenderer()->GetWidth(), (hFloat)GetRenderer()->GetHeight()); !!JM

            // !!JM culled client side work
            if (mainRender_ && runnableState)
                (*mainRender_)( this );

            // GetRenderer()->rendererFrameSubmit(); !!JM
            /*
             * Frame isn't swapped until next call to EndRenderFrame() so
             * can render to back buffer here
             **/
            //debugMenuManager_->RenderMenus( !!JM
            //    GetRenderer()->GetMainSubmissionCtx(), 
            //    GetRenderer()->GetMaterialManager());
            /*
             * Swap back buffer and Submit to GPU draw calls sent to renderer in mainRender()
             **/
            // GetRenderer()->EndRenderFrame(); !! JM
            
            getActionManager()->endOfFrameUpdate();
        }

#ifdef HEART_DO_PROFILE
        g_ProfilerManager_->SetFrameTime(frameTimer.elapsedMicroSec()/1000.f);
#endif
        //debugMenuManager_->EndFrameUpdate(); !!JM
    }

    hHeartEngine::~hHeartEngine() {
        mem_track_marker("PreEngineShutdown");//hMemTracking::TrackPopMarker();

        //debugMenuManager_->Destroy(); !!JM
        //console_->destroy(); !!JM
        //!JM todo: soundManager_->Destory(); 
        hResourceManager::shutdown();
        luaVM_->Destroy();
        hRenderer::destroy();
        debugServer_->destroy();
        jobManager->shutdown();

        //delete GetGlobalHeap(), debugInfo_; GetGlobalHeap(), debugInfo_ = nullptr;
        //delete debugMenuManager_; debugMenuManager_ = nullptr; !!JM
        delete luaVM_; luaVM_ = nullptr;
        //delete console_; console_ = nullptr; !!JM
        //!!JM todo: delete soundManager_; soundManager_ = nullptr;
        delete actionManager_; actionManager_ = nullptr;
        delete fileMananger_; fileMananger_ = nullptr;
        delete jobManager; jobManager = nullptr;
        delete mainPublisherCtx_; mainPublisherCtx_ = nullptr;
        delete system_; system_ = nullptr;
        delete debugServer_; debugServer_ = nullptr;

        hNetwork::shutdown();

        // Delete all global objects allocated by libprotobuf.
        google::protobuf::ShutdownProtobufLibrary();

        hDestroyFreetype2();
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
                if (hResourceManager::systemResourcesReady()) {
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
        ImGuiInit();
#if HEART_DEBUG
#pragma message ("TODO- Real time profiler menu")
/*        hRTProfilerMenu* rtProfileMenu_ = new GetDebugHeap(), hRTProfilerMenu)(debugMenuManager_->GetDebugCanvas(), GetRenderer();
        rtProfileMenu_->SetHidden(hTrue);
        debugMenuManager_->RegisterMenu("rtp", rtProfileMenu_);

        hMemoryViewMenu* mvMenu_ = new GetDebugHeap(), hMemoryViewMenu)(debugMenuManager_->GetDebugCanvas();
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
#if defined (HEART_PLAT_WINDOWS)
Heart::hHeartEngine* HEART_API hHeartInitEngine(hHeartEngineCallbacks* callbacks, const hChar* config_script, hSize_t script_len, int argc, char** argv)
{
    //Heart::hSysCall::hInitSystemDebugLibs();
    heart_thread_prof_begin("profile_startup.prof");
    Heart::hdDeviceConfig deviceConfig;

    Heart::hHeartEngine* engine = new Heart::hHeartEngine(config_script, script_len);

    engine->firstLoaded_        = callbacks->firstLoaded_;
    engine->coreAssetsLoaded_   = callbacks->coreAssetsLoaded_;
    engine->mainUpdate_         = callbacks->mainUpdate_;
    engine->mainRender_         = callbacks->mainRender_;
    engine->shutdownUpdate_     = callbacks->shutdownUpdate_;
    engine->onShutdown_         = callbacks->onShutdown_;

    // !!JM culled client side work
    // if (engine->firstLoaded_)
    // {
    //     (*engine->firstLoaded_)(engine);
    // }

    heart_thread_prof_end(0);
    return engine;
}
#elif defined (HEART_PLAT_LINUX)
Heart::hHeartEngine* HEART_API hHeartInitEngine(hHeartEngineCallbacks* callbacks) {
    Heart::hdDeviceConfig deviceConfig;
    heart_thread_prof_begin("profile_startup.prof");

    Heart::hHeartEngine* engine = 
        new Heart::hHeartEngine(callbacks->overrideFileRoot_, callbacks->consoleCallback_, callbacks->consoleCallbackUser_, &deviceConfig);

    engine->firstLoaded_        = callbacks->firstLoaded_;
    engine->coreAssetsLoaded_   = callbacks->coreAssetsLoaded_;
    engine->mainUpdate_         = callbacks->mainUpdate_;
    engine->mainRender_         = callbacks->mainRender_;
    engine->shutdownUpdate_     = callbacks->shutdownUpdate_;
    engine->onShutdown_         = callbacks->onShutdown_;

    // !!JM culled client side work
    // if (engine->firstLoaded_)
    // {
    //     (*engine->firstLoaded_)(engine);
    // }

    heart_thread_prof_end(0);
    return engine;
}
#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

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

void HEART_API hHeartShutdownEngine( Heart::hHeartEngine* engine )
{
    //Heart::hd_CloseSharedLib(engine->sharedLib_);

    delete engine;
}
