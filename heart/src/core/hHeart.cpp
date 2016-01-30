/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "Heart.h"
#include "2d/hTileRenderer2D.h"
#include "lua/hLuaHeartLib.h"
#include "render/hImGuiRenderer.h"
#include "render/hMaterial.h"
#include "render/hUniformBufferResource.h"
#include "render/hRenderPipeline.h"
#include "render/hView.h"
#include "memtracker.h"

#define HEART_PRINT_FRAME_START_END 0

namespace Heart {
    hChar           hHeartEngine::HEART_VERSION_STRING[] = "HeartEngine v0.4";
    const hFloat    hHeartEngine::HEART_VERSION = 0.4f;
    hUint g_RenderFenceCount = 3;

    static hStringID g_DefaultFrameTaskGraph("default_frame_graph");
    static hStringID sync_task_id("heart::SYNC");
    static hStringID clock_update_task_id("heart::clock::update");
    static hStringID debug_server_service_task_id("heart::debug_server::service");
    static hStringID resource_mgr_update_task_id("heart::resource_manager::update");
    static hStringID action_mgr_update_task_id("heart::action_manager::update");
    static hStringID lua_vm_update_task_id("heart::lua_vm::update");
    static hStringID evt_publiser_update_task_id("heart::event_publisher::update");
    static hStringID action_mgr_end_frame_task_id("heart::action_manager::end_of_frame_update");
    static hStringID update_debug_menus_task_id("heart::debug_menus::update");
    static hStringID main_frame_update_task_id("heart::main_frame_update");// placeholder
    static hStringID main_frame_render_task_id("heart::main_frame_render");// placeholder

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
        fileMananger_->mountPoint(hConfigurationVariables::getCVarStr("filesystem.scripts_dir", "/script"), "/scripts");
        fileMananger_->mountPoint(hConfigurationVariables::getCVarStr("filesystem.data_dir", "/data"), "/data" );
        fileMananger_->mountPoint(hConfigurationVariables::getCVarStr("filesystem.tmp_dir", "/tmp"), "/tmp");
        fileMananger_->mountPoint(hConfigurationVariables::getCVarStr("filesystem.save_dir", "/save"), "/save");

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
        actionManager_ = new hActionManager;
        system_ = new hSystem;
        soundManager_ = nullptr;//new hSoundManager;
        console_ = nullptr;//new hSystemConsole(consoleCb, consoleUser);
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
        hTaskScheduler::initialise(hConfigurationVariables::getCVarUint("taskgraph.workercount", hSysCall::getProcessorCount()), hConfigurationVariables::getCVarUint("taskgraph.jobqueuesize", 256));
        debugServer_->initialise(hConfigurationVariables::getCVarInt("debug.server.port", 8335));
        mainPublisherCtx_->initialise(1024*1024);
        system_->Create(config_);
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
        hResourceManager::initialise(fileMananger_);
        hTileRenderer2D::initialise();
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

        // Workaround for dead stripping of these types
        hBool register_type = hTrue;
        register_type &= hShaderProgram::auto_object_registered;
        register_type &= hTTFFontFace::auto_object_registered;
        register_type &= hTextureResource::auto_object_registered;
        register_type &= hTextureAtlasResource::auto_object_registered;
        register_type &= hLevel::auto_object_registered;
        register_type &= hTileRenderer2D::registerComponents();
        register_type &= hMaterial::auto_object_registered;
        register_type &= hUniformBufferResource::auto_object_registered;
        register_type &= hRenderingPipeline::auto_object_registered;
        if (register_type) {
            engineState_ = hHeartState_LoadingCore;
        }

        //Setup the standard task graph to call when no custom task graph is installed.
        registerEngineTasks();
        createFrameTaskGraph();
        mem_track_marker("AfterEngineInit");
    }

    void hHeartEngine::registerEngineTasks() {
        hTaskFactory::registerTask(clock_update_task_id, [&](hTaskInfo* i) { hClock::update(); return 0; });
        hTaskFactory::registerTask(debug_server_service_task_id, [&](hTaskInfo* i) { debugServer_->service(); return 0; });
        hTaskFactory::registerTask(resource_mgr_update_task_id, [&](hTaskInfo* i) { hResourceManager::update(); return 0; });
        hTaskFactory::registerTask(action_mgr_update_task_id, [&](hTaskInfo* i) { actionManager_->update(); return 0; });
        hTaskFactory::registerTask(sync_task_id, [](hTaskInfo* i) {return 0; });
        hTaskFactory::registerTask(lua_vm_update_task_id, [&](hTaskInfo* i) { luaVM_->Update(); return 0; });
        hTaskFactory::registerTask(evt_publiser_update_task_id, [&](hTaskInfo* i) { mainPublisherCtx_->updateDispatch(); return 0; });
        hTaskFactory::registerTask(action_mgr_end_frame_task_id, [&](hTaskInfo* i) { actionManager_->endOfFrameUpdate(); return 0; });
        hTaskFactory::registerTask(main_frame_update_task_id, [&](hTaskInfo*) {
            if (engineState_ == hHeartState_Running || engineState_ == hHeartState_Paused)
                (*mainUpdate_)(this);
        });
        hTaskFactory::registerTask(main_frame_render_task_id, [&](hTaskInfo*) {
            if (engineState_ == hHeartState_Running || engineState_ == hHeartState_Paused)
                (*mainRender_)(this);
        });
        hTaskFactory::registerTask(update_debug_menus_task_id, [&](hTaskInfo*) {
            hDebugMenuManager::SubmitMenus(actionManager_);
        });
        hRenderer::registerViewTasks();

        hTaskFactory::registerNamedTaskGraph(g_DefaultFrameTaskGraph);
    }

    void hHeartEngine::createFrameTaskGraph() {
        hBool resourcesReady = hResourceManager::systemResourcesReady();
        mainFrameTaskGraph = hTaskFactory::getNamedTaskGraph(g_DefaultFrameTaskGraph);
        mainFrameTaskGraph->clear();
        submitFrameTaskHandle.reset();
        hTaskFactory::setActiveTaskGraph(g_DefaultFrameTaskGraph);
        auto clock_update_task = mainFrameTaskGraph->addTask(clock_update_task_id);
        auto db_server_service = mainFrameTaskGraph->addTask(debug_server_service_task_id);
        auto res_update = mainFrameTaskGraph->addTask(resource_mgr_update_task_id);
        auto action_update = mainFrameTaskGraph->addTask(action_mgr_update_task_id);
        auto sync1 = mainFrameTaskGraph->addTask(sync_task_id);
        auto vm_update = mainFrameTaskGraph->addTask(lua_vm_update_task_id);
        auto sync2 = mainFrameTaskGraph->addTask(sync_task_id);
        auto publisher_context = mainFrameTaskGraph->addTask(evt_publiser_update_task_id);
        auto sync3 = mainFrameTaskGraph->addTask(sync_task_id);
        auto action_eof = mainFrameTaskGraph->addTask(action_mgr_end_frame_task_id);

        mainFrameTaskGraph->createTaskDependency(clock_update_task, db_server_service);
        mainFrameTaskGraph->createTaskDependency(clock_update_task, action_update);
        mainFrameTaskGraph->createTaskDependency(clock_update_task, res_update);
        mainFrameTaskGraph->createTaskDependency(res_update, sync1);
        mainFrameTaskGraph->createTaskDependency(sync1, vm_update);
        mainFrameTaskGraph->createTaskDependency(vm_update, sync2);
        mainFrameTaskGraph->createTaskDependency(sync2, publisher_context);
        mainFrameTaskGraph->createTaskDependency(publisher_context, sync3);
        mainFrameTaskGraph->createTaskDependency(sync3, action_eof);

        if (resourcesReady) {
            auto main_update_task = mainFrameTaskGraph->addTask(main_frame_update_task_id);
            auto main_render_task = mainFrameTaskGraph->addTask(main_frame_render_task_id);
            auto debug_menu_update = mainFrameTaskGraph->addTask(update_debug_menus_task_id);
            auto initialise_views_for_frame = mainFrameTaskGraph->addTask(hRenderer::initialise_views_for_frame_task);
            auto sort_views_for_frame = mainFrameTaskGraph->addTask(hRenderer::sort_views_for_frame_task);
            auto submit_views = mainFrameTaskGraph->addTask(hRenderer::submit_views_task);
            submitFrameTaskHandle = mainFrameTaskGraph->addTask(hRenderer::submit_frame_task);
            mainFrameTaskGraph->createTaskDependency(sync1, main_update_task);
            mainFrameTaskGraph->createTaskDependency(main_update_task, publisher_context);
            mainFrameTaskGraph->createTaskDependency(main_update_task, debug_menu_update);
            mainFrameTaskGraph->createTaskDependency(debug_menu_update, sync3);
            mainFrameTaskGraph->createTaskDependency(sync3, initialise_views_for_frame);
            mainFrameTaskGraph->createTaskDependency(initialise_views_for_frame, main_render_task);
            mainFrameTaskGraph->createTaskDependency(main_render_task, sort_views_for_frame);
            mainFrameTaskGraph->createTaskDependency(sort_views_for_frame, submit_views);
            mainFrameTaskGraph->createTaskDependency(submit_views, submitFrameTaskHandle);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hHeartEngine::DoUpdate() {
        hcCondPrintf(HEART_PRINT_FRAME_START_END, "Frame Begin");
#ifdef HEART_DO_PROFILE
        g_ProfilerManager_->BeginFrame();
#endif
        hTimer frameTimer;
        frameTimer.reset();
        system_->Update();
        mainFrameTaskGraph = hTaskFactory::getActiveTaskGraph();
        hRenderer::hCmdList* main_cl = nullptr;
        if (submitFrameTaskHandle.isValid()) {
            main_cl = hRenderer::createCmdList();
            mainFrameTaskGraph->clearTaskInputs(submitFrameTaskHandle);
            mainFrameTaskGraph->addTaskInput(submitFrameTaskHandle, main_cl);
        }
        mainFrameTaskGraph->kick();
        mainFrameTaskGraph->wait();
        if (main_cl) {
            hRenderer::submitFrame(main_cl);
        }
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
#ifdef HEART_DO_PROFILE
        g_ProfilerManager_->SetFrameTime(frameTimer.elapsedMicroSec()/1000.f);
#endif
        //debugMenuManager_->EndFrameUpdate(); !!JM
        hcCondPrintf(HEART_PRINT_FRAME_START_END, "Frame End");
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

        //delete GetGlobalHeap(), debugInfo_; GetGlobalHeap(), debugInfo_ = nullptr;
        //delete debugMenuManager_; debugMenuManager_ = nullptr; !!JM
        delete luaVM_; luaVM_ = nullptr;
        //delete console_; console_ = nullptr; !!JM
        //!!JM todo: delete soundManager_; soundManager_ = nullptr;
        delete actionManager_; actionManager_ = nullptr;
        delete fileMananger_; fileMananger_ = nullptr;
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
                    if (coreAssetsLoaded_) {
                        (*coreAssetsLoaded_)(this);
                    }
                    // re-create the task graph, it will add extra jobs because main resources are now loaded.
                    createFrameTaskGraph();
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

    Heart::hStringID hHeartEngine::GetDefaultFrameTaskGraphName() {
        return g_DefaultFrameTaskGraph;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hHeartEngine::PostCoreResourceLoad() {
        auto* pipeline = hResourceManager::weakResource<hRenderingPipeline>(hStringID("/system/test_render_pipeline"));
        hRenderer::reinitialiseViews(*pipeline);
        ImGuiInit();
        hTileRenderer2D::initialiseResources();
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
