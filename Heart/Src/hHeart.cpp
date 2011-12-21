/********************************************************************
	created:	2008/06/08
	created:	8:6:2008   20:23
	filename: 	Heart.cpp
	author:		James Moran
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hHeart.h"
#include "hMath.h"
#include "hUtility.h"
#include "hResourceManager.h"
#include "hEventManager.h"
#include "hControllerManager.h"
#include "hSystem.h"
#include "hFont.h"
#include "hSystemConsole.h"
#include "hLuaStateManager.h"
#include "hJobManager.h"
#include "hClock.h"
#include "hZipFileSystem.h"
#include "hDriveFileSystem.h"
#include "hSceneGraph.h"
#include "hDebugRenderer.h"
#include "hConfigOptions.h"

/************************************************************************/
/*
	Engine Memory Heap allocations
*/
/************************************************************************/
#define MB				( 1024 * 1024 )
hMemoryHeap hDebugHeap(			0   * MB, false );
hMemoryHeap hRendererHeap(			2	* MB, false );
hMemoryHeap hResourceHeap(			10	* MB, false );
hMemoryHeap hSceneGraphHeap(		0	* MB, false );
hMemoryHeap hGeneralHeap(			1	* MB, false );
hMemoryHeap hVMHeap(				0	* MB, false );

/************************************************************************/
/*	Engine Init                                                         */
/************************************************************************/

namespace Heart
{
	hChar			HeartEngine::HEART_VERSION_STRING[] = "HeartEngine v0.4";
	const hFloat	HeartEngine::HEART_VERSION = 0.4f;
	HeartEngine*	HeartEngine::pInstance_ = NULL;

	HeartEngine* InitHeartEngine( HeartConfig& Config, const char* configFile )
	{
		Config.pEngine_ = hNEW ( hGeneralHeap ) HeartEngine( Config, configFile );
		return Config.pEngine_;
	}


	HeartEngine::HeartEngine(  HeartConfig& config, const char* configFile )
	{
		pInstance_ = this;

		//strcpy_s( pHomeDirectory_, HOME_DIRECTORY_MAX_LEN, config.pWorkingDir_ );

		//
		// Create engine classes
		//
		eventManager_ = hNEW ( hGeneralHeap ) EventManager();

		jobManager_ = hNEW ( hGeneralHeap ) hJobManager();

		controllerManager_ = hNEW ( hGeneralHeap ) hControllerManager();

		system_ = hNEW( hGeneralHeap ) hSystem();

		fileMananger_ = hNEW ( hGeneralHeap ) hDriveFileSystem();

		zipFileSystem_ = hNEW ( hGeneralHeap ) hZipFileSystem();

		resourceMananger_ = hNEW ( hGeneralHeap ) hResourceManager();

		renderer_ = hNEW ( hGeneralHeap ) hRenderer();
		
		sceneGraph_ = hNEW ( hGeneralHeap ) hSceneGraph();

		console_ = hNEW ( hGeneralHeap ) hSystemConsole();

		luaVM_ = hNEW ( hGeneralHeap ) hLuaStateManager();

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
// 			"engine/fonts/console.fnt",
// 			"engine/materials/basic2dtex.mat",
// 			"engine/materials/consoleback.mat",
// 			"engine/materials/directionlight.mat",
// 			"engine/materials/directionlightshadow.mat",
// 			"engine/materials/shadowwrite.mat",
// 			"engine/materials/pointlight1st.mat",
// 			"engine/materials/pointlight2nd.mat",
// 			"engine/materials/pointlightinside.mat",
// 			"engine/materials/spotlight1st.mat",
// 			"engine/materials/spotlight2nd.mat",
// 			"engine/materials/spotlightshadow2nd.mat",
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
// 
// 		resourceMananger_->SetResourceHandlers( "ixb",
// 			hResourceManager::ResourceLoadCallback::bind< hRenderer, &hRenderer::OnIndexBufferLoad >( renderer_ ), 
// 			hResourceManager::ResourceUnloadCallback::bind< hRenderer, &hRenderer::OnIndexBufferUnload >( renderer_ ),
// 			NULL );
// 
// 		resourceMananger_->SetResourceHandlers( "vxb",
// 			hResourceManager::ResourceLoadCallback::bind< hRenderer, &hRenderer::OnVertexBufferLoad >( renderer_ ), 
// 			hResourceManager::ResourceUnloadCallback::bind< hRenderer, &hRenderer::OnVertexBufferUnload >( renderer_ ),
// 			NULL );
// 
// 		resourceMananger_->SetResourceHandlers( "msh",
// 			hResourceManager::ResourceLoadCallback::bind< hRenderer, &hRenderer::OnMeshLoad >( renderer_ ), 
// 			hResourceManager::ResourceUnloadCallback::bind< hRenderer, &hRenderer::OnMeshUnload >( renderer_ ),
// 			NULL );
// 
// 		resourceMananger_->SetResourceHandlers( "scn",
// 			hResourceManager::ResourceLoadCallback::bind< hSceneGraph, &hSceneGraph::OnSceneLoad >( sceneGraph_ ),
// 			hResourceManager::ResourceUnloadCallback::bind< hSceneGraph, &hSceneGraph::OnSceneUnload >( sceneGraph_ ), 
// 			NULL );
// 
// 		resourceMananger_->SetResourceHandlers( "sgn",
// 			hResourceManager::ResourceLoadCallback::bind< hSceneGraph, &hSceneGraph::OnSceneNodeLoad >( sceneGraph_ ),
// 			hResourceManager::ResourceUnloadCallback::bind< hSceneGraph, &hSceneGraph::OnSceneNodeUnload >( sceneGraph_ ), 
// 			NULL );
// 
// 		resourceMananger_->SetResourceHandlers( "loc",
// 			hResourceManager::ResourceLoadCallback::bind< hSceneGraph, &hSceneGraph::OnSceneNodeLocatorLoad >( sceneGraph_ ),
// 			hResourceManager::ResourceUnloadCallback::bind< hSceneGraph, &hSceneGraph::OnSceneNodeUnload >( sceneGraph_ ), 
// 			NULL );
// 
// 		resourceMananger_->SetResourceHandlers( "lig",
// 			hResourceManager::ResourceLoadCallback::bind< hSceneGraph, &hSceneGraph::OnSceneNodeLightLoad >( sceneGraph_ ),
// 			hResourceManager::ResourceUnloadCallback::bind< hSceneGraph, &hSceneGraph::OnSceneNodeUnload >( sceneGraph_ ), 
// 			NULL );
// 
// 		resourceMananger_->SetResourceHandlers( "cam",
// 			hResourceManager::ResourceLoadCallback::bind< hSceneGraph, &hSceneGraph::OnSceneNodeCameraLoad >( sceneGraph_ ),
// 			hResourceManager::ResourceUnloadCallback::bind< hSceneGraph, &hSceneGraph::OnSceneNodeUnload >( sceneGraph_ ), 
// 			NULL );
// 
// 		resourceMananger_->SetResourceHandlers( "mhc",
// 			hResourceManager::ResourceLoadCallback::bind< hSceneGraph, &hSceneGraph::OnSceneNodeMeshCollectionLoad >( sceneGraph_ ),
// 			hResourceManager::ResourceUnloadCallback::bind< hSceneGraph, &hSceneGraph::OnSceneNodeUnload >( sceneGraph_ ), 
// 			NULL );

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

		resourceMananger_->Initialise( fileMananger_, requiredResourcesList );

		sceneGraph_->Initialise( resourceMananger_ );

		hIFileSystem* luaFilesystems[] = 
		{
			//TODO: Add disk file system
			//TODO: Add TCP/IP file system
			//zipFileSystem_,
			fileMananger_,
			NULL
		};

		luaVM_->Initialise( luaFilesystems );

		//////////////////////////////////////////////////////////////////////////
		// Console needs resources, call after setup functions ///////////////////
		//////////////////////////////////////////////////////////////////////////

		console_->Initialise( controllerManager_, luaVM_, resourceMananger_, renderer_ );

		DebugRenderer::Initialise( resourceMananger_, renderer_ );

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// Initialise Engine scripting elements ///////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// static methods ////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// methods ///////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

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
			// Engine Setup is now complete, Let the game does its Init steps ////////
			// The following could be moved elsewhere as its is pretty cross /////////
			// platform //////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			config->initFunc_( lpCmdLine, pEngine );

			//////////////////////////////////////////////////////////////////////////
			// Game Loop - again could be elsewhere //////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			for(;;)
			{
				pEngine->GetSystem()->Update();

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
		delete pEngine;

		// Check memory usage
#define CHECK_HEAP( x ) hcAssertMsg( x.BytesAllocated() == 0, "Heap "#x" leaking %u bytes", x.BytesAllocated() );

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
		//jobManager_->Destory();

		DebugRenderer::Destory();

		console_->Destroy();

		sceneGraph_->Destroy(); 

		resourceMananger_->Shutdown( renderer_ );

		renderer_->StopRenderThread();

		zipFileSystem_->Destory();

		luaVM_->Destroy();

		renderer_->Destroy();

		delete luaVM_;

		delete zipFileSystem_;

		delete console_;

		delete sceneGraph_;

		delete renderer_;

		delete controllerManager_;

		delete resourceMananger_;

		delete fileMananger_;

		delete jobManager_;

		delete eventManager_;

		delete system_;
		
	}

}