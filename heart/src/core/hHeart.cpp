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

#include "Heart.h"
#include "lua/hLuaHeartLib.h"

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

    hHeartEngine::hHeartEngine(const hChar* rootdir, hConsoleOutputProc consoleCb, void* consoleUser, hdDeviceConfig* deviceConfig)
        : firstLoaded_(hNullptr)
        , coreAssetsLoaded_(hNullptr)
        , mainUpdate_(hNullptr)
        , mainRender_(hNullptr)
        , shutdownUpdate_(hNullptr)
        , onShutdown_(hNullptr)
    {
        GOOGLE_PROTOBUF_VERIFY_VERSION;

        hUint32 sdlFlags = 0;
        sdlFlags |= SDL_INIT_VIDEO;
        sdlFlags |= SDL_INIT_GAMECONTROLLER;
        sdlFlags |= SDL_INIT_HAPTIC;
        sdlFlags |= SDL_INIT_EVENTS;
        SDL_Init(sdlFlags);

        hdGetCurrentWorkingDir(workingDir_.GetBuffer(), workingDir_.GetMaxSize());
        hdGetProcessDirectory(processDir_.GetBuffer(), processDir_.GetMaxSize());

        hcPrintf("Current Directory: %s\nProcess Directory: %s", workingDir_.GetBuffer(), processDir_.GetBuffer());

        hdMountPoint(processDir_, "proc");
        hdMountPoint(workingDir_, "cd");

        {
            char tmp[1024];
            hcAssert(hdGetSystemPathSize("/proc/scripts") < 1024);
            hdGetSystemPath("/proc/scripts", tmp, 1024);
            hdMountPoint(tmp, "script");

            hcAssert(hdGetSystemPathSize("/proc/data") < 1024);
            hdGetSystemPath("/proc/data", tmp, 1024);
            hdMountPoint(tmp, "data");

            hcAssert(hdGetSystemPathSize("/proc/tmp") < 1024);
            hdGetSystemPath("/proc/tmp", tmp, 1024);
            hdMountPoint(tmp, "tmp");

            hcAssert(hdGetSystemPathSize("/proc/save") < 1024);
            hdGetSystemPath("/proc/save", tmp, 1024);
            hdMountPoint(tmp, "save");
        }

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
        mainPublisherCtx_ = new hPublisherContext;
        jobManager_ = new hJobManager;
        actionManager_ = new hActionManager;
        system_ = new hSystem;
        fileMananger_ = new hDriveFileSystem;
        soundManager_ = nullptr;//new hSoundManager;
        console_ = nullptr;//new hSystemConsole(consoleCb, consoleUser);
        luaVM_ = new hLuaStateManager;
        debugMenuManager_ = nullptr;//new hDebugMenuManager;
        debugServer_=new hNetHost;

        //////////////////////////////////////////////////////////////////////////
        // Read in the configFile_ ///////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        luaVM_->Initialise();
        hConfigurationVariables::loadCVars(luaVM_->GetMainState(), fileMananger_ );

        config_.Width_ = hConfigurationVariables::getCVarUint("renderer.width", 640);
        config_.Height_ = hConfigurationVariables::getCVarUint("renderer.height", 480);
        config_.Fullscreen_ = hConfigurationVariables::getCVarBool("renderer.fullscreen", false);
        config_.vsync_ = hConfigurationVariables::getCVarBool("renderer.vsync", false);

        deviceConfig_ = deviceConfig;

        //////////////////////////////////////////////////////////////////////////
        // Init Engine Classes ///////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        
        //hcSetOutputStringCallback(hSystemConsole::printConsoleMessage);
        debugServer_->initialise(hConfigurationVariables::getCVarInt("debug.server.port", 8335));
        mainPublisherCtx_->initialise(1024*1024);
        system_->Create(config_, deviceConfig_);
        jobManager_->initialise();
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
        hResourceManager::initialise(fileMananger_, jobManager_);
        //!!JM todo: soundManager_->Initialise();

        ///////////////////////////////////////////////////////////////////////////////////////////////////
        // Initialise Engine scripting elements ///////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////

        OpenHeartLuaLib(luaVM_->GetMainState(), this);
        actionManager_->registerLuaLib(luaVM_);

        //Run the start up script
        const hChar* script_name = hConfigurationVariables::getCVarStr("startup.script", nullptr);
        if (script_name) {
            if (hIFile* startup_script = fileMananger_->OpenFile(script_name, FILEMODE_READ)) {
                hChar* script=nullptr;
                if (startup_script->getIsMemMapped()) {
                    script=(hChar*)startup_script->getMemoryMappedBase();
                } else {
                    script = (hChar*)hAlloca((hSize_t)startup_script->Length()+1);
                    startup_script->Read(script, (hUint)startup_script->Length());
                }
                luaL_loadbuffer(luaVM_->GetMainState(), script, startup_script->Length(), script_name);
                if (lua_pcall(luaVM_->GetMainState(), 0, LUA_MULTRET, 0) != 0) {
                    hcAssertFailMsg("startup.lua Failed to run, Error: %s", lua_tostring(luaVM_->GetMainState(), -1));
                    lua_pop(luaVM_->GetMainState(), 1);
                }
                fileMananger_->CloseFile(startup_script);
            }
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
        engineState_ = hHeartState_LoadingCore;

        // Temp workaround for dead stripping of these types
        hcPrintf("Auto-registered type %s=%d", hShaderProgram::getTypeNameStatic().c_str(), hShaderProgram::auto_object_registered);
        hcPrintf("Auto-registered type %s=%d", hMaterial::getTypeNameStatic().c_str(), hMaterial::auto_object_registered);

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
            hResourceManager::update();
            getActionManager()->update();
            //GetConsole()->update(); !!JM
            if (GetSystem()->exitSignaled()) {
                /*if (!shutdownUpdate_ || shutdownUpdate_(this))*/ {
                    //wait on game to say ok to shutdown
                    //GetJobManager()->Destory(); // Should this be here?
                    // if (onShutdown_ && runnableState) onShutdown_(this); !!JM 
                    engineState_ = hHeartState_ShuttingDown;
                    return;
                }
            }
            //!!JM todo: GetSoundManager()->Update();
            // !!JM culled client side work
            //if (mainUpdate_ && runnableState)
            //    (*mainUpdate_)( this );
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
            //if (mainRender_ && runnableState)
            //    (*mainRender_)( this );

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

    hHeartEngine::~hHeartEngine()
    {
        hMemTracking::TrackPopMarker();

        //debugMenuManager_->Destroy(); !!JM
        //console_->destroy(); !!JM
        //!JM todo: soundManager_->Destory(); 
        hResourceManager::shutdown();
        luaVM_->Destroy();
        hRenderer::destroy();
        debugServer_->destroy();
        jobManager_->shutdown();

        //delete GetGlobalHeap(), debugInfo_; GetGlobalHeap(), debugInfo_ = nullptr;
        //delete debugMenuManager_; debugMenuManager_ = nullptr; !!JM
        delete luaVM_; luaVM_ = nullptr;
        //delete console_; console_ = nullptr; !!JM
        //!!JM todo: delete soundManager_; soundManager_ = nullptr;
        delete actionManager_; actionManager_ = nullptr;
        delete fileMananger_; fileMananger_ = nullptr;
        delete jobManager_; jobManager_ = nullptr;
        delete mainPublisherCtx_; mainPublisherCtx_ = nullptr;
        delete system_; system_ = nullptr;
        delete debugServer_; debugServer_ = nullptr;

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
Heart::hHeartEngine* HEART_API hHeartInitEngine(hHeartEngineCallbacks* callbacks, HINSTANCE hInstance, HWND hWnd)
{
    Heart::hSysCall::hInitSystemDebugLibs();
    heart_thread_prof_begin("profile_startup.prof");
    Heart::hdDeviceConfig deviceConfig;

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
#elif defined (HEART_PLAT_LINUX)
Heart::hHeartEngine* HEART_API hHeartInitEngine(hHeartEngineCallbacks* callbacks)
{
    Heart::hSysCall::hInitSystemDebugLibs();
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
