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

extern "C"
{
#ifdef _WIN32
    HEARTCORE_SLIBEXPORT Heart::HeartEngine* HEART_API hHeartInitEngine( const hChar*, HINSTANCE hInstance );
#else
    #error ("Platform not supported")
#endif
    HEARTCORE_SLIBEXPORT hUint32 HEART_API hHeartDoMainUpdate( Heart::HeartEngine* );
    HEARTCORE_SLIBEXPORT void HEART_API hHeartShutdownEngine( Heart::HeartEngine* );
};

namespace Heart
{

    class hDriveFileSystem;
    class hZipFileSystem;
    class hJobManager;
    class hResourceManager;
    class hSystemConsole;
    class hSoundManager;
    class EventManager;
    class hControllerManager;
    class hSystem;
    class hRenderer;
    class hSceneGraph;
    class hLuaStateManager;
    class hEntityFactory;
    class HeartEngine;

    typedef void(*FirstLoaded)(HeartEngine*);
    typedef void(*CoreAssetsLoaded)(HeartEngine*);
    typedef void(*MainUpdate)(HeartEngine*);
    typedef void(*MainRender)(HeartEngine*);
    typedef hBool(*ShutdownUpdate)(HeartEngine*);
    typedef void(*OnShutdown)(HeartEngine*);

    class HEARTCORE_SLIBEXPORT HeartConfig
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

        HeartEngine*        pEngine_;
    };

    enum hHeartState
    {
        hHeartState_LoadingCore,
        hHeartState_Running,
        hHeartState_Paused,
        hHeartState_Finised,
        hHeartState_FatalError = -1,
    };

    class HEARTCORE_SLIBEXPORT HeartEngine
    {
    public:

        HeartEngine( const char* configFile, hdDeviceConfig* deviceConfig );
        ~HeartEngine();

        hDriveFileSystem*   GetFileManager()        { return fileMananger_; }
        hSystem*            GetSystem()             { return system_; }
        hRenderer*          GetRenderer()           { return renderer_; }
        hResourceManager*   GetResourceManager()    { return resourceMananger_; }
        EventManager*       GetEventManager()       { return eventManager_; }
        hControllerManager* GetControllerManager()  { return controllerManager_; }
        hSystemConsole*     GetConsole()            { return console_; }
        hLuaStateManager*   GetVM()                 { return luaVM_; }
        hJobManager*        GetJobManager()         { return jobManager_; }
        hEntityFactory*     GetEntityFactory()      { return entityFactory_; }
        hSoundManager*      GetSoundManager()       { return soundManager_; }
        hGwenRenderer*      GetUIRenderer()         { return uiRenderer_; }
        void*               GetUIMenuManager()      { return NULL; }

        static const hChar* VersionString()         { return HEART_VERSION_STRING; }
        static hFloat       Version()               { return HEART_VERSION; }

        void                DoEngineTick();
        hHeartState         GetState()              { return engineState_; }

    private:

        friend HEARTCORE_SLIBEXPORT HeartEngine* HEART_API hHeartInitEngine(const hChar*);

        static hChar         HEART_VERSION_STRING[];
        static const hFloat  HEART_VERSION; 
        static const hUint32 HEART_VERSION_MAJOR = 0;
        static const hUint32 HEART_VERSION_MINOR = 4;


        void                            RegisterDefaultComponents();
        void                            PostCoreResourceLoad();
        void                            DoUpdate();
        void                            DoUpdateNoApp();

        HeartConfig                     config_;
        hdDeviceConfig                  deviceConfig_;
        hHeartState                     engineState_;
        
        hDriveFileSystem*               fileMananger_;
        hResourceManager*               resourceMananger_;
        hSystem*                        system_;
        hRenderer*                      renderer_;
        hSoundManager*                  soundManager_;
        EventManager*                   eventManager_;
        hControllerManager*             controllerManager_;
        hSystemConsole*                 console_;
        hLuaStateManager*               luaVM_;
        hJobManager*                    jobManager_;
        hEntityFactory*                 entityFactory_;
        hGwenRenderer*                  uiRenderer_;
        hConfigOptions                  configFile_;

        //Debug 
        hDebugMenuManager*              debugMenuManager_;

        //app functions
    public:
        hSharedLibAddress               sharedLib_;
        FirstLoaded                     firstLoaded_;
        CoreAssetsLoaded                coreAssetsLoaded_;
        MainUpdate                      mainUpdate_;
        MainRender                      mainRender_;
        ShutdownUpdate                  shutdownUpdate_;
        OnShutdown                      onShutdown_;
    };
}



#endif // _DEBUG
