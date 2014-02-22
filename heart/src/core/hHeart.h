/********************************************************************

filename:     hHeart.h    

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

#ifndef HEART_ENGINE_H__
#define HEART_ENGINE_H__

namespace Heart
{
    class hHeartEngine;
}

typedef void(*hFirstLoadedProc)(Heart::hHeartEngine*);
typedef void(*hCoreAssetsLoadedProc)(Heart::hHeartEngine*);
typedef void(*hMainUpdateProc)(Heart::hHeartEngine*);
typedef void(*hMainRenderProc)(Heart::hHeartEngine*);
typedef hBool(*hShutdownUpdateProc)(Heart::hHeartEngine*);
typedef void(*hOnShutdownProc)(Heart::hHeartEngine*);

struct hHeartEngineCallbacks
{
    hFirstLoadedProc                     firstLoaded_;
    hCoreAssetsLoadedProc                coreAssetsLoaded_;
    hMainUpdateProc                      mainUpdate_;
    hMainRenderProc                      mainRender_;
    hShutdownUpdateProc                  shutdownUpdate_;
    hOnShutdownProc                      onShutdown_;
    const hChar*                         overrideFileRoot_;
    hConsoleOutputProc                   consoleCallback_;
    void*                                consoleCallbackUser_;
};

extern "C"
{
#ifdef WIN32

    HEART_DLLEXPORT Heart::hHeartEngine* HEART_API hHeartInitEngineFromSharedLib(const hChar*, HINSTANCE hInstance, HWND hWnd);
    HEART_DLLEXPORT Heart::hHeartEngine* HEART_API hHeartInitEngine(hHeartEngineCallbacks*, HINSTANCE hInstance, HWND hWnd);
#else
#   error ("Platform not supported")
#endif
    HEART_DLLEXPORT hUint32 HEART_API hHeartDoMainUpdate( Heart::hHeartEngine* );
    HEART_DLLEXPORT void HEART_API hHeartShutdownEngine( Heart::hHeartEngine* );
};

namespace Heart
{

    class hDriveFileSystem;
    class hZipFileSystem;
    class hJobManager;
    class hResourceManager;
    class hSystemConsole;
    class hSoundManager;
    class hPublisherContext;
    class hControllerManager;
    class hSystem;
    class hRenderer;
    class hSceneGraph;
    class hLuaStateManager;
    class hEntityFactory;
    class hHeartEngine;

    template < typename _ty >
    class hGlobalFinaliser
    {
    public:
        ~hGlobalFinaliser() {
            for (auto i=objectList_.begin(), n=objectList_.end(); i!=n; ++i) {
                hDELETE_SAFE(*i);
            }
        }

        void addObject(_ty* object) {
            hMutexAutoScope mas(&lock_);
            objectList_.push_back(object);
        }

    private:

        hdMutex            lock_;
        std::vector<_ty*> objectList_;
    };

    class HEART_DLLEXPORT HeartConfig
    {
    public:

        HeartConfig() 
            : bpp_( 32 )
            , MinShaderVersion_( 3.0 )
            , pEngine_( NULL )
        {}

        hdDeviceConfig      deviceConfig_;

        hUint32             Width_;
        hUint32             Height_;
        hUint32             bpp_;
        hBool               Fullscreen_;
        hBool               vsync_;
        hFloat              MinShaderVersion_;

        hHeartEngine*        pEngine_;
    };

    enum hHeartState
    {
        hHeartState_LoadingCore,
        hHeartState_Running,
        hHeartState_Paused,
        hHeartState_ShuttingDown,
        hHeartState_Finised = hErrorCode,
    };

    class HEART_DLLEXPORT hHeartEngine
    {
    public:

        hHeartEngine(const hChar* rootdir, hConsoleOutputProc consoleCb, void* consoleUser, hdDeviceConfig* deviceConfig);
        ~hHeartEngine();

        hIFileSystem*       GetFileManager()        { return fileMananger_; }
        hSystem*            GetSystem()             { return system_; }
        hRenderer*          GetRenderer()           { return renderer_; }
        hResourceManager*   GetResourceManager()    { return resourceMananger_; }
        hPublisherContext*  GetMainEventPublisher() { return mainPublisherCtx_; }
        hControllerManager* GetControllerManager()  { return controllerManager_; }
        hSystemConsole*     GetConsole()            { return console_; }
        hLuaStateManager*   GetVM()                 { return luaVM_; }
        hJobManager*        GetJobManager()         { return jobManager_; }
        hEntityFactory*     GetEntityFactory()      { return entityFactory_; }
        hSoundManager*      GetSoundManager()       { return soundManager_; }

        static const hChar* VersionString();
        static hFloat       Version();
        const hChar*        GetWorkingDir() { return workingDir_.GetBuffer(); }
        void                DoEngineTick();
        hHeartState         GetState() { return engineState_; }

    private:

        friend HEART_DLLEXPORT hHeartEngine* HEART_API hHeartInitEngineFromSharedLib(const hChar*);

        static hChar         HEART_VERSION_STRING[];
        static const hFloat  HEART_VERSION; 
        static const hUint32 HEART_VERSION_MAJOR = 0;
        static const hUint32 HEART_VERSION_MINOR = 4;
        static hdW32ThreadEvent exitSignal_;

        static void                     ProtoBufLogHandler(google::protobuf::LogLevel level, const char* filename, int line, const std::string& message);

        void                            RegisterDefaultComponents();
        void                            PostCoreResourceLoad();
        void                            DoUpdate();

        HeartConfig                     config_;
        hdDeviceConfig                  deviceConfig_;
        hHeartState                     engineState_;
        hArray< hChar, 1024 >           workingDir_;
        hArray< hChar, 1024 >           processDir_;
        
        hIFileSystem*                   fileMananger_;
        hResourceManager*               resourceMananger_;
        hSystem*                        system_;
        hRenderer*                      renderer_;
        hSoundManager*                  soundManager_;
        hPublisherContext*              mainPublisherCtx_;
        hControllerManager*             controllerManager_;
        hSystemConsole*                 console_;
        hLuaStateManager*               luaVM_;
        hJobManager*                    jobManager_;
        hEntityFactory*                 entityFactory_;
        hConfigOptions                  configFile_;

        //Debug 
        hDebugMenuManager*              debugMenuManager_;
        hDebugInfo*                     debugInfo_;
        hNetHost*                       debugServer_;

        //app functions
    public:
        hSharedLibAddress               sharedLib_;
        hFirstLoadedProc                     firstLoaded_;
        hCoreAssetsLoadedProc                coreAssetsLoaded_;
        hMainUpdateProc                      mainUpdate_;
        hMainRenderProc                      mainRender_;
        hShutdownUpdateProc                  shutdownUpdate_;
        hOnShutdownProc                      onShutdown_;
    };
}



#endif // _DEBUG