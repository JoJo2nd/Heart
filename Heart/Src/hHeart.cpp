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
#define MB                ( 1024 * 1024 )

Heart::hMemoryHeap hDebugHeap(       0 * MB, false );
Heart::hMemoryHeap hRendererHeap(    2 * MB, false );
Heart::hMemoryHeap hResourceHeap(   10 * MB, false );
Heart::hMemoryHeap hSceneGraphHeap(  0 * MB, false );
Heart::hMemoryHeap hGeneralHeap(     1 * MB, false );
Heart::hMemoryHeap hVMHeap(          0 * MB, false );

/************************************************************************/
/*    Engine Init                                                         */
/************************************************************************/

namespace Heart
{
    hChar            HeartEngine::HEART_VERSION_STRING[] = "HeartEngine v0.4";
    const hFloat    HeartEngine::HEART_VERSION = 0.4f;
    HeartEngine*    HeartEngine::pInstance_ = NULL;

    HeartEngine* InitHeartEngine( HeartConfig& Config, const char* configFile )
    {
        Config.pEngine_ = hNEW ( hGeneralHeap, HeartEngine( Config, configFile ) );
        return Config.pEngine_;
    }


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HeartEngine::HeartEngine(  HeartConfig& config, const char* configFile )
    {
        pInstance_ = this;

        //strcpy_s( pHomeDirectory_, HOME_DIRECTORY_MAX_LEN, config.pWorkingDir_ );

        //
        // Create engine classes
        //
        eventManager_ = hNEW(hGeneralHeap, EventManager);

        jobManager_ = hNEW_ALIGN(hGeneralHeap, 32, hJobManager);

        controllerManager_ = hNEW(hGeneralHeap , hControllerManager);

        system_ = hNEW(hGeneralHeap, hSystem);

        fileMananger_ = hNEW(hGeneralHeap, hDriveFileSystem);

        zipFileSystem_ = hNEW(hGeneralHeap, hZipFileSystem);

        resourceMananger_ = hNEW(hGeneralHeap, hResourceManager);

        renderer_ = hNEW(hGeneralHeap, hRenderer);

        sceneGraph_ = hNEW(hGeneralHeap, hSceneGraph);

        soundManager_ = hNEW(hGeneralHeap, hSoundManager);

        console_ = hNEW(hGeneralHeap, hSystemConsole);

        luaVM_ = hNEW(hGeneralHeap, hLuaStateManager);

        entityFactory_ = hNEW(hGeneralHeap, hEntityFactory);

        //////////////////////////////////////////////////////////////////////////
        // Read in the configFile_ ////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        if ( configFile )
            configFile_.ReadConfig( configFile, fileMananger_ );
        else
            configFile_.ReadConfig( "config.cfg", fileMananger_ );

        config.Width_ = configFile_.GetScreenWidth();
        config.Height_ = configFile_.GetScreenHeight();
        config.Fullscreen_ = configFile_.GetFullscreen();
        config.vsync_ = configFile_.GetVsync();

#ifdef HEART_PLAT_WINDOWS
        config.deviceConfig_.Width_ = config.Width_;
        config.deviceConfig_.Height_ = config.Height_;
#endif

        ///////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        //TODO: Get these from the game + engine list
        const char*  requiredResourcesList[] =
        {
            "ENGINE/EFFECTS/DEBUG.CFX",
            "ENGINE/FONTS/CONSOLE.FNT",
            "ENGINE/EFFECTS/SIMPLECOLOUR.CFX",
            NULL
        };

        ///////////////////////////////////////////////////////////////////////////////////////////////////
        // Register Engine side resource loaders //////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////

        resourceMananger_->SetResourceHandlers( "TEX", 
            hResourceManager::ResourceLoadCallback::bind< hRenderer, &hRenderer::OnTextureLoad >( renderer_ ), 
            hResourceManager::ResourceUnloadCallback::bind< hRenderer, &hRenderer::OnTextureUnload >( renderer_ ),
            NULL );

        resourceMananger_->SetResourceHandlers( "FNT",
            hResourceManager::ResourceLoadCallback::bind< &hFont::OnFontLoad >(),
            hResourceManager::ResourceUnloadCallback::bind< &hFont::OnFontUnload >(),
            NULL );

        resourceMananger_->SetResourceHandlers( "CFX",
            hResourceManager::ResourceLoadCallback::bind< hRenderer, &hRenderer::OnMaterialLoad >( renderer_ ), 
            hResourceManager::ResourceUnloadCallback::bind< hRenderer, &hRenderer::OnMaterialUnload >( renderer_ ),
            NULL );

        resourceMananger_->SetResourceHandlers( "GPU",
            hResourceManager::ResourceLoadCallback::bind< hRenderer, &hRenderer::OnShaderProgramLoad >( renderer_ ), 
            hResourceManager::ResourceUnloadCallback::bind< hRenderer, &hRenderer::OnShaderProgramUnload >( renderer_ ),
            NULL );

        resourceMananger_->SetResourceHandlers( "OGG",
            hResourceManager::ResourceLoadCallback::bind< &hSoundResource::OnSoundLoad >(), 
            hResourceManager::ResourceUnloadCallback::bind< &hSoundResource::OnSoundUnload >(),
            NULL );

        resourceMananger_->SetResourceHandlers( "SBK",
            hResourceManager::ResourceLoadCallback::bind< &hSoundBankResource::OnSoundBankLoad >(), 
            hResourceManager::ResourceUnloadCallback::bind< &hSoundBankResource::OnSoundBankUnload >(),
            NULL );

        resourceMananger_->SetResourceHandlers( "WOD", 
            hResourceManager::ResourceLoadCallback::bind< hEntityFactory, &hEntityFactory::OnWorldObjectScriptLoad >(entityFactory_),
            hResourceManager::ResourceUnloadCallback::bind< hEntityFactory, &hEntityFactory::OnWorldObjectScriptUnload >(entityFactory_),
            NULL );

        //////////////////////////////////////////////////////////////////////////
        // Register Engine Level Event Channels //////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        eventManager_->AddChannel( KERNEL_EVENT_CHANNEL );

        //////////////////////////////////////////////////////////////////////////
        // Init Engine Classes ///////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        system_->Create( config, eventManager_ );

        jobManager_->Initialise();

        controllerManager_->Initialise( eventManager_ );

        hClock::Initialise();

        zipFileSystem_->Initialise( config.pGamedataPak_ );

        renderer_->Create( 
            system_,
            config.Width_,
            config.Height_,
            config.bpp_,
            config.MinShaderVersion_,
            config.Fullscreen_,
            config.vsync_,
            resourceMananger_
            );

        resourceMananger_->Initialise( renderer_, fileMananger_, requiredResourcesList );

        sceneGraph_->Initialise( resourceMananger_ );

        soundManager_->Initialise();

        hIFileSystem* luaFilesystems[] = 
        {
            //TODO: Add disk file system
            //TODO: Add TCP/IP file system
            //zipFileSystem_,
            fileMananger_,
            NULL
        };

        luaVM_->Initialise( luaFilesystems );

        entityFactory_->Initialise( fileMananger_, resourceMananger_, pInstance_ );

        //////////////////////////////////////////////////////////////////////////
        // Console needs resources, call after setup functions ///////////////////
        //////////////////////////////////////////////////////////////////////////

        console_->Initialise( controllerManager_, luaVM_, resourceMananger_, renderer_ );

        DebugRenderer::Initialise( resourceMananger_, renderer_ );

        ///////////////////////////////////////////////////////////////////////////////////////////////////
        // Initialise Engine scripting elements ///////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////

        RegisterDefaultComponents();

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

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    void HeartRunMainLoop( HeartConfig* config, Heart::HeartEngine* pEngine, LPSTR lpCmdLine, hBool* quitFlag )
    {

        hClock::Update();

        //////////////////////////////////////////////////////////////////////////
        // Engine Init Loop //////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        do//while( !pEngine->GetResourceManager()->RequiredResourcesReady() )
        {
            pEngine->GetSystem()->Update();

            pEngine->GetResourceManager()->MainThreadUpdate();

            //before calling Update dispatch the last frames messages
            pEngine->GetEventManager()->DispatchEvents();

            hClock::Update();

            pEngine->GetConsole()->Update();

            pEngine->GetControllerManager()->Update();

            if ( (*quitFlag) )
            {
                pEngine->GetJobManager()->Destory();
                break;
            }

            pEngine->GetSoundManager()->Update();

            pEngine->GetVM()->Update();

            pEngine->GetRenderer()->BeginRenderFrame( hTrue );

            pEngine->GetConsole()->Render( pEngine->GetRenderer() );

            pEngine->GetRenderer()->EndRenderFrame();

        } 
        while( !pEngine->GetResourceManager()->RequiredResourcesReady() );

        // Quit request in start up?
        if ( !(*quitFlag) )
        {
            //////////////////////////////////////////////////////////////////////////
            // Engine Setup is now complete, Let the game do its Init steps //////////
            //////////////////////////////////////////////////////////////////////////
            config->initFunc_( lpCmdLine, pEngine );

            //////////////////////////////////////////////////////////////////////////
            // Game Loop /////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////
            for(;;)
            {
                pEngine->GetSystem()->Update();

                pEngine->GetResourceManager()->MainThreadUpdate();

                //before calling Update dispatch the last frames messages
                pEngine->GetEventManager()->DispatchEvents();

                hClock::Update();

                pEngine->GetConsole()->Update();

                pEngine->GetControllerManager()->Update();

                if ( (*quitFlag) )
                {
                    if ( config->shutdownTickFunc_( pEngine ) )
                    {
                        //wait on game to say ok to shutdown
                        pEngine->GetJobManager()->Destory();
                        break;
                    }
                }

                pEngine->GetSoundManager()->Update();

                config->updateFunc_( hClock::Delta(), pEngine );

                pEngine->GetVM()->Update();

                pEngine->GetRenderer()->BeginRenderFrame( hTrue );

                config->renderFunc_( hClock::Delta(), pEngine );

                pEngine->GetConsole()->Render( pEngine->GetRenderer() );

                pEngine->GetRenderer()->EndRenderFrame();

            } 
        }

        //////////////////////////////////////////////////////////////////////////
        // Allow game to clean up ////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        config->postShutdownFunc_( pEngine );
    }

    void ShutdownHeartEngine(  HeartEngine* pEngine  )
    {
        hDELETE(hGeneralHeap, pEngine);

        // Check memory usage
#define CHECK_HEAP( x ) x.printLeaks(#x)//hcAssertMsg( x.bytesAllocated() == 0, "Heap "#x" leaking %u bytes", x.bytesAllocated() );

        CHECK_HEAP( hGeneralHeap );
        CHECK_HEAP( hRendererHeap );
        CHECK_HEAP( hResourceHeap );
        CHECK_HEAP( hSceneGraphHeap );
        CHECK_HEAP( hVMHeap );

#undef CHECK_HEAP

    }

    HeartEngine::~HeartEngine()
    {
        eventManager_->RemoveChannel( KERNEL_EVENT_CHANNEL );

        // job manager is destroyed by the quit logic
        jobManager_->Destory();

        DebugRenderer::Destory();

        console_->Destroy();

        soundManager_->Destory();

        sceneGraph_->Destroy(); 

        resourceMananger_->Shutdown( renderer_ );

        renderer_->StopRenderThread();

        zipFileSystem_->Destory();

        luaVM_->Destroy();

        renderer_->Destroy();

        hDELETE_SAFE(hGeneralHeap, entityFactory_);

        hDELETE(hGeneralHeap, luaVM_);

        hDELETE(hGeneralHeap, zipFileSystem_);

        hDELETE(hGeneralHeap, console_);

        hDELETE(hGeneralHeap, soundManager_);

        hDELETE(hGeneralHeap, sceneGraph_);

        hDELETE(hGeneralHeap, renderer_);

        hDELETE(hGeneralHeap, controllerManager_);

        hDELETE(hGeneralHeap, resourceMananger_);

        hDELETE(hGeneralHeap, fileMananger_);

        hDELETE_ALIGNED(hGeneralHeap, jobManager_);

        hDELETE(hGeneralHeap, eventManager_);

        hDELETE(hGeneralHeap, system_);

    }
}