/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef HEART_ENGINE_H__
#define HEART_ENGINE_H__

#include "base/hTypes.h"
#include "base/hArray.h"
#include "base/hFunctor.h"
#include "base/hMutex.h"
#include "base/hMutexAutoScope.h"
#include "base/hSysCalls.h"
#include "core/hProtobuf.h"
#include "core/hConfigOptions.h"
#include "core/hSystemConsole.h"
#include "core/hDeviceConfig.h"
#include "core/hDeviceSystemWindow.h"

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
     Heart::hHeartEngine* HEART_API hHeartInitEngine(hHeartEngineCallbacks*, const hChar*, hSize_t, int argc, char** argv);
     hUint32 HEART_API hHeartDoMainUpdate( Heart::hHeartEngine* );
     void HEART_API hHeartShutdownEngine( Heart::hHeartEngine* );
};

namespace Heart
{

    class hActionManager;
    class hDebugInfo;
    class hDebugMenuManager;
    class hDriveFileSystem;
    class hHeartEngine;
    class hIFileSystem;    
    class hJobManager;
    class hLuaStateManager;
    class hPublisherContext;
    class hNetHost;
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
            : deviceConfig_(nullptr)
            , bpp_( 32 )
            , MinShaderVersion_( 3.0 )
            , pEngine_(nullptr)
        {}

        hdDeviceConfig*     deviceConfig_;

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

        hHeartEngine(const hChar* config_script, hSize_t config_script_len);
        ~hHeartEngine();

        hIFileSystem*       GetFileManager()        { return fileMananger_; }
        hSystem*            GetSystem()             { return system_; }
        hPublisherContext*  GetMainEventPublisher() { return mainPublisherCtx_; }
        hActionManager*     getActionManager()      { return actionManager_; }
        hSystemConsole*     GetConsole()            { return console_; }
        hLuaStateManager*   GetVM()                 { return luaVM_; }
        hJobManager*        GetJobManager()         { return jobManager; }
        hSoundManager*      GetSoundManager()       { return soundManager_; }

        static const hChar* VersionString();
        static hFloat       Version();
        const hChar*        GetWorkingDir() { return workingDir_.GetBuffer(); }
        void                DoEngineTick();
        hHeartState         GetState() { return engineState_; }

    private:

        static hChar         HEART_VERSION_STRING[];
        static const hFloat  HEART_VERSION; 
        static const hUint32 HEART_VERSION_MAJOR = 0;
        static const hUint32 HEART_VERSION_MINOR = 4;

        static void                     ProtoBufLogHandler(google::protobuf::LogLevel level, const char* filename, int line, const std::string& message);

        void                            PostCoreResourceLoad();
        void                            DoUpdate();

        HeartConfig                     config_;
        hdDeviceConfig*                 deviceConfig_;
        hHeartState                     engineState_;
        hArray< hChar, 1024 >           workingDir_;
        hArray< hChar, 1024 >           processDir_;
        
        hIFileSystem*                   fileMananger_;
        hSystem*                        system_;
        hSoundManager*                  soundManager_;
        hPublisherContext*              mainPublisherCtx_;
        hActionManager*                 actionManager_;
        hSystemConsole*                 console_;
        hLuaStateManager*               luaVM_;
        hJobManager*                    jobManager;

        //Debug 
        hDebugMenuManager*              debugMenuManager_;
        hDebugInfo*                     debugInfo_;
        hNetHost*                       debugServer_;

        //app functions
    public:
//         hSharedLibAddress               sharedLib_;
        hFirstLoadedProc                     firstLoaded_;
        hCoreAssetsLoadedProc                coreAssetsLoaded_;
        hMainUpdateProc                      mainUpdate_;
        hMainRenderProc                      mainRender_;
        hShutdownUpdateProc                  shutdownUpdate_;
        hOnShutdownProc                      onShutdown_;
    };
}



#endif // _DEBUG
