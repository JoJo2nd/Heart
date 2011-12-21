#ifndef HEART_ENGINE_H__
#define HEART_ENGINE_H__


#include "hHeartConfig.h"
#include "hDebugMacros.h"
#include "Common.h"
#include "hTypes.h"
#include "hMemory.h"
#include "huFunctor.h"
#include "hConfigOptions.h"

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

		hDriveFileSystem*				fileMananger_;
		hZipFileSystem*					zipFileSystem_;
		hResourceManager*				resourceMananger_;
		hSystem*						system_;
		hRenderer*						renderer_;
		hSceneGraph*						sceneGraph_;
		EventManager*					eventManager_;
		hControllerManager*				controllerManager_;
		hSystemConsole*					console_;
		hLuaStateManager*				luaVM_;
		hJobManager*					jobManager_;
		hChar							pHomeDirectory_[ HOME_DIRECTORY_MAX_LEN ];
		hConfigOptions					configFile_;
	};
}

//////////////////////////////////////////////////////////////////////////
// Heart Task List ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*
	TODO: 0.0 Implement all features in this list. Syntax for items in this list.
	TODO:{space}{number.number} then any other sounce files for this feature can use the 
	same to do label. The first number is used to identify/group tasks logically, the 2nd number
	is the task number in the task group
	This is a very ad hoc list but its kinda nice to have it all here an quite clear.
	Note that all to do in this list should have a clear description of the work needed.
*/
/*
	To do group 1 is mainly to do with coding style
*/
/*
	TODO: 1.1 Remove all double nested name spaces within the engine. 
	e.g. Heart::Core::Clock is to be group under just the Heart name space. so 
	Heart::Clock, Heart::hResourceManager, Heart::Renderer. The only exception to this rule 
	might be the Render Commands which keep the Cmd name space. But I'm not fully sure about 
	this yet.
*/
/*
	TODO: 1.2 Remove all OLD h* prefixes on classes and rename all files to match 
	the main class/interface defined in the header.
*/
/*
	TODO: 1.3 Add h prefixes to all classes in HEART. 
	e.g. Heart::Renderer would become Heart::hRenderer and Heart::Matrix would be Heart::hMatrix
*/
/*
	TODO: 1.4 Change all defined to use HEART_ e.g. DEBUG becomes HEART_DEBUG
*/
/*
	TODO: 1.5 Change all macros to use h prefix e.g max(x,y) becomes hMax()
*/
/*
	TODO: 1.6 Change all variables to use lower case first letter, CamelCase and underscore afterwards e.g. memberVar_
*/
/*
	TODO: 1.7 Change all funtions to be CamelCase e.g. hClass::MemberFuncion() or GlobalFunction();
*/
/*
	TODO: 1.8 Change enums to use enum type name as first element of enum name
	e.g.
	enum SomeEnum 
	{
		SomeEnum_A_ENUM_VALUE
	}
*/
/*
	to do group 2 is mainly to do with CORE engine functionality. e.g. start up, device layer
*/
/*
	TODO: 2.1 Change all new overloads within the engine to use a macro hNEW which simply 
	uses placement new, but keep delete as it is and use a hDELETE macro.
*/
/*
	TODO: 2.2 Reduce down to a single allocator with a small pool allocator which sits atop this
	large dl malloc allocator.
*/
/*
	TODO: 2.3 Restructure engine lib to be used as a DLL library. Allow engine to run the main loop as it
	does now, but also allow engine main loop to be stepped.
*/
/*
	TODO: 2.4 Move over to use STLport instead of MS STL.
*/
/*
	TODO: 2.5 Restructure engine to load device layer and renderer as a separate DLL library to the main Heart engine.
*/
/*
	TODO: 2.6 Remove Interfaces from Device layer
*/
/*
	TODO: 2.7 Change enums for render state to directly map to device (OpenGL or DirectX) values
*/
/*
	TODO: 2.8 Move TinyXML into its own library
*/
/*
	to do group 3 is mainly to do with Resource loading/unloaded systems
*/
/*
	TODO: 3.1 Remove the ResourceHandle< _Ty > class and replace with a base ResourceClass. This should remove most of the layers
	of indirection in the resource manager class. Resources will initially be stored in a linked list class. Afterwards a Red-Black Tree
	might be used. 
*/
/*
	TODO: 3.2 Change all resource classes to support template based reflection.
*/
/*
	TODO: 3.3 Design a system to provide runtime swapping of resources due to external changes to the file. Initially this will not 
	be done while a resource is loaded but the final system should attempt this.
*/
/*
	to do group 4 is mainly to do with render systems.
*/
/*
	TODO: 4.1 Remove D3DX Effect system. Reduce the material system to only need vertex & fragment shaders, not effects.
	Fix texture sampler registers look up. Add Passes (Possibly). Add Material HASH ID system.
*/

#endif // _DEBUG
