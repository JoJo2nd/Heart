#ifndef HEART_ENGINE_H__
#define HEART_ENGINE_H__


#include "hHeartConfig.h"
#include "hDebugMacros.h"
#include "Common.h"
#include "hTypes.h"
#include "hMemory.h"
#include "huFunctor.h"
#include "hConfigOptions.h"
#include "hEntity.h"
#include "hComponent.h"

namespace Heart
{

	class hDriveFileSystem;
	class hZipFileSystem;
	class hJobManager;
	class hResourceManager;
	class hSystemConsole;
	class EventManager;
	class hControllerManager;
	class hSystem;					
	class hRenderer;
	class hSceneGraph;
	class hLuaStateManager;
    class hEntityFactory;
	class HeartEngine;

	typedef huFunctor< void (*)(const hChar*, HeartEngine*) >::type		PostEngineInit;
	typedef huFunctor< void (*)(hFloat, HeartEngine*) >::type			OnUpdate;
	typedef huFunctor< void (*)(hFloat, HeartEngine*) >::type			OnRender;
	typedef huFunctor< bool (*)(HeartEngine*) >::type					OnShutdownTick;
	typedef huFunctor< void (*)(HeartEngine*) >::type					PostEngineShutdown;

	static const hUint32			HOME_DIRECTORY_MAX_LEN = 2048;
	static const hUint32			WNDCLASSNAMELEN = 256;

	class HeartConfig
	{
	public:

		explicit HeartConfig( HeartConfig* pConfig ) { *this = *pConfig; }

		HeartConfig( 
			hFloat MinShaderVersion, 
			const hChar* pGamedataPak,
			PostEngineInit initFunc,
			OnUpdate updateFunc, 
			OnRender renderFunc, 
			OnShutdownTick shutdownTick,
			PostEngineShutdown postShutdown
			) 
				: bpp_( 32 )
				, MinShaderVersion_( MinShaderVersion )
				, pGamedataPak_( pGamedataPak )
				, initFunc_( initFunc )
				, updateFunc_( updateFunc )
				, renderFunc_( renderFunc )
				, shutdownTickFunc_( shutdownTick )
				, postShutdownFunc_( postShutdown )
				, pEngine_( NULL )
		{}

		hdDeviceConfig	deviceConfig_;

		hUint32			Width_;
		hUint32			Height_;
		hUint32			bpp_;
		hBool			Fullscreen_;
		hBool			vsync_;
		hFloat			MinShaderVersion_;
		const char*		pGamedataPak_;
		PostEngineInit	initFunc_;
		OnUpdate		updateFunc_;
		OnRender		renderFunc_;
		OnShutdownTick  shutdownTickFunc_;
		PostEngineShutdown	postShutdownFunc_;
		
		HeartEngine*	pEngine_;
	};

	Heart::HeartEngine*&  SetupHeartEngineConfigParams( HeartConfig& config );
	HeartEngine* InitHeartEngine( HeartConfig&, const char* );
	void HeartRunMainLoop( HeartConfig* config, Heart::HeartEngine* pEngine, LPSTR lpCmdLine, hBool* quitFlag );
	void ShutdownHeartEngine( HeartEngine* );

	class HeartEngine
	{
	public:

		HeartEngine( HeartConfig& pConfig, const char* configFile );
		~HeartEngine();

		hDriveFileSystem*				GetFileManager()		{ return fileMananger_; }
		hSystem*						GetSystem()				{ return system_; }
		hRenderer*						GetRenderer()			{ return renderer_; }
		hResourceManager*				GetResourceManager()	{ return resourceMananger_; }
		hSceneGraph*					GetSceneGraph()			{ return sceneGraph_; }
		EventManager*					GetEventManager()		{ return eventManager_; }
		hControllerManager*				GetControllerManager()	{ return controllerManager_; }
		hSystemConsole*					GetConsole()			{ return console_; }
		hLuaStateManager*				GetVM()					{ return luaVM_; }
		hJobManager*					GetJobManager()			{ return jobManager_; }
        hEntityFactory*                 GetEntityFactory()      { return entityFactory_; }
		const hChar*					GetHomeDirectory()		{ return pHomeDirectory_; }
	
		static const hChar*				VersionString()			{ return HEART_VERSION_STRING; }
		static hFloat					Version()				{ return HEART_VERSION; }
		static hUint32					GetMemoryUsage()		{ return hVMHeap.BytesAllocated(); }
		static HeartEngine*				Instance()				{ return pInstance_; }

	private:

		static hChar					HEART_VERSION_STRING[];
		static const hFloat				HEART_VERSION; 
		static const hUint32			HEART_VERSION_MAJOR = 0;
		static const hUint32			HEART_VERSION_MINOR = 4;
		static HeartEngine*				pInstance_;

        void                            RegisterDefaultComponents();

		hDriveFileSystem*				fileMananger_;
		hZipFileSystem*					zipFileSystem_;
		hResourceManager*				resourceMananger_;
		hSystem*						system_;
		hRenderer*						renderer_;
		hSceneGraph*					sceneGraph_;
		EventManager*					eventManager_;
		hControllerManager*				controllerManager_;
		hSystemConsole*					console_;
		hLuaStateManager*				luaVM_;
		hJobManager*					jobManager_;
        hEntityFactory*                 entityFactory_;
		hChar							pHomeDirectory_[ HOME_DIRECTORY_MAX_LEN ];
		hConfigOptions					configFile_;
	};
}

#endif // _DEBUG
