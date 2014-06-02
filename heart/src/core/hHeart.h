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

#include "base/hTypes.h"
#include "base/hArray.h"
#include "base/hFunctor.h"
#include "base/hProtobuf.h"
#include "core/hConfigOptions.h"
#include "core/hSystemConsole.h"
#include "pal/hMutex.h"
#include "pal/hEvent.h"
#include "pal/hDeviceConfig.h"
#include "pal/hDeviceSystemWindow.h"

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

     Heart::hHeartEngine* HEART_API hHeartInitEngineFromSharedLib(const hChar*, HINSTANCE hInstance, HWND hWnd);
     Heart::hHeartEngine* HEART_API hHeartInitEngine(hHeartEngineCallbacks*, HINSTANCE hInstance, HWND hWnd);
#else
#   error ("Platform not supported")
#endif
     hUint32 HEART_API hHeartDoMainUpdate( Heart::hHeartEngine* );
     void HEART_API hHeartShutdownEngine( Heart::hHeartEngine* );
};

namespace Heart
{

    class hActionManager;
    class hDebugInfo;
    class hDebugMenuManager;
    class hDriveFileSystem;
    class hEntityFactory;
    class hHeartEngine;
    class hIFileSystem;    
    class hJobManager;
    class hLuaStateManager;
    class hPublisherContext;
    class hRenderer;   
    class hSceneGraph;
    class hSoundManager;
    class hSystem;
    class hSystemConsole;
    class hSystemConsole;  
    class hZipFileSystem;

    template < typename _ty >
    class hGlobalFinaliser
    {
    public:
        ~hGlobalFinaliser() {
            for (auto i=objectList_.begin(), n=objectList_.end(); i!=n; ++i) {
                delete *i; *i = nullptr;
            }
        }

        void addObject(_ty* object) {
            hMutexAutoScope mas(&lock_);
            objectList_.push_back(object);
        }

    private:

        hMutex            lock_;
        std::vector<_ty*> objectList_;
    };

    class  HeartConfig
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

    class  hHeartEngine
    {
    public:

        hHeartEngine(const hChar* rootdir, hConsoleOutputProc consoleCb, void* consoleUser, hdDeviceConfig* deviceConfig);
        ~hHeartEngine();

        hIFileSystem*       GetFileManager()        { return fileMananger_; }
        hSystem*            GetSystem()             { return system_; }
        hRenderer*          GetRenderer()           { return renderer_; }
        hPublisherContext*  GetMainEventPublisher() { return mainPublisherCtx_; }
        hActionManager*     getActionManager()      { return actionManager_; }
        hSystemConsole*     GetConsole()            { return console_; }
        hLuaStateManager*   GetVM()                 { return luaVM_; }
        hJobManager*        GetJobManager()         { return jobManager_; }
        hSoundManager*      GetSoundManager()       { return soundManager_; }

        static const hChar* VersionString();
        static hFloat       Version();
        const hChar*        GetWorkingDir() { return workingDir_.GetBuffer(); }
        void                DoEngineTick();
        hHeartState         GetState() { return engineState_; }

    private:

        friend  hHeartEngine* HEART_API hHeartInitEngineFromSharedLib(const hChar*);

        static hChar         HEART_VERSION_STRING[];
        static const hFloat  HEART_VERSION; 
        static const hUint32 HEART_VERSION_MAJOR = 0;
        static const hUint32 HEART_VERSION_MINOR = 4;
        static hThreadEvent exitSignal_;

        static void                     ProtoBufLogHandler(google::protobuf::LogLevel level, const char* filename, int line, const std::string& message);

        void                            PostCoreResourceLoad();
        void                            DoUpdate();

        HeartConfig                     config_;
        hdDeviceConfig                  deviceConfig_;
        hHeartState                     engineState_;
        hArray< hChar, 1024 >           workingDir_;
        hArray< hChar, 1024 >           processDir_;
        
        hIFileSystem*                   fileMananger_;
        hSystem*                        system_;
        hRenderer*                      renderer_;
        hSoundManager*                  soundManager_;
        hPublisherContext*              mainPublisherCtx_;
        hActionManager*                 actionManager_;
        hSystemConsole*                 console_;
        hLuaStateManager*               luaVM_;
        hJobManager*                    jobManager_;
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
