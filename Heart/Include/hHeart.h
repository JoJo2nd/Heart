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

    typedef huFunctor< void (*)(const hChar*, HeartEngine*) >::type        PostEngineInit;
    typedef huFunctor< void (*)(hFloat, HeartEngine*) >::type            OnUpdate;
    typedef huFunctor< void (*)(hFloat, HeartEngine*) >::type            OnRender;
    typedef huFunctor< bool (*)(HeartEngine*) >::type                    OnShutdownTick;
    typedef huFunctor< void (*)(HeartEngine*) >::type                    PostEngineShutdown;

    static const hUint32            HOME_DIRECTORY_MAX_LEN = 2048;
    static const hUint32            WNDCLASSNAMELEN = 256;

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

        hdDeviceConfig    deviceConfig_;

        hUint32             Width_;
        hUint32             Height_;
        hUint32             bpp_;
        hBool               Fullscreen_;
        hBool               vsync_;
        hFloat              MinShaderVersion_;
        const char*         pGamedataPak_;
        PostEngineInit      initFunc_;
        OnUpdate            updateFunc_;
        OnRender            renderFunc_;
        OnShutdownTick      shutdownTickFunc_;
        PostEngineShutdown  postShutdownFunc_;

        HeartEngine*    pEngine_;
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

        hDriveFileSystem*   GetFileManager()        { return fileMananger_; }
        hSystem*            GetSystem()             { return system_; }
        hRenderer*          GetRenderer()           { return renderer_; }
        hResourceManager*   GetResourceManager()    { return resourceMananger_; }
        hSceneGraph*        GetSceneGraph()         { return sceneGraph_; }
        EventManager*       GetEventManager()       { return eventManager_; }
        hControllerManager* GetControllerManager()  { return controllerManager_; }
        hSystemConsole*     GetConsole()            { return console_; }
        hLuaStateManager*   GetVM()                 { return luaVM_; }
        hJobManager*        GetJobManager()         { return jobManager_; }
        hEntityFactory*     GetEntityFactory()      { return entityFactory_; }
        hSoundManager*      GetSoundManager()       { return soundManager_; }
        hGwenRenderer*      GetUIRenderer()         { return uiRenderer_; }
        void*               GetUIMenuManager()      { return NULL; }
        const hChar*        GetHomeDirectory()      { return pHomeDirectory_; }

        static const hChar* VersionString()         { return HEART_VERSION_STRING; }
        static hFloat       Version()               { return HEART_VERSION; }

    private:

        static hChar         HEART_VERSION_STRING[];
        static const hFloat  HEART_VERSION; 
        static const hUint32 HEART_VERSION_MAJOR = 0;
        static const hUint32 HEART_VERSION_MINOR = 4;

        void                            RegisterDefaultComponents();

        hDriveFileSystem*   fileMananger_;
        hZipFileSystem*     zipFileSystem_;
        hResourceManager*   resourceMananger_;
        hSystem*            system_;
        hRenderer*          renderer_;
        hSceneGraph*        sceneGraph_;
        hSoundManager*      soundManager_;
        EventManager*       eventManager_;
        hControllerManager* controllerManager_;
        hSystemConsole*     console_;
        hLuaStateManager*   luaVM_;
        hJobManager*        jobManager_;
        hEntityFactory*     entityFactory_;
        hGwenRenderer*      uiRenderer_;
        hChar               pHomeDirectory_[ HOME_DIRECTORY_MAX_LEN ];
        hConfigOptions      configFile_;
    };
}

#endif // _DEBUG
