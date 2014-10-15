/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "testbed_precompiled.h"
#include "TestBedCore.h"
#include "resource_texture.pb.h"

TestBedCore g_TestCore;

void HEART_API HeartAppFirstLoaded(Heart::hHeartEngine*)
{}

void HEART_API HeartAppCoreAssetsLoaded(Heart::hHeartEngine* engine) 
{
    g_TestCore.Initialise(NULL, engine);
}

void HEART_API HeartAppMainUpate(Heart::hHeartEngine* engine)
{
    g_TestCore.EngineUpdateTick(Heart::hClock::delta(), engine);
}

void HEART_API HeartAppMainRender(Heart::hHeartEngine* engine)
{
    g_TestCore.EngineRenderTick(Heart::hClock::delta(), engine);
}

hBool HEART_API HeartAppShutdownUpdate(Heart::hHeartEngine* engine)
{
    return g_TestCore.EngineShutdownRequest(engine);
}

void HEART_API HeartAppOnShutdown(Heart::hHeartEngine* engine)
{
    g_TestCore.EngineShutdown(engine);
}

#if defined (PLATFORM_WINDOWS)

void CreateErrorBox( DWORD error );

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{	
    hHeartEngineCallbacks callbacks = {0};

    callbacks.firstLoaded_ = HeartAppFirstLoaded;
    callbacks.coreAssetsLoaded_ = HeartAppCoreAssetsLoaded;
    callbacks.mainUpdate_ = HeartAppMainUpate;
    callbacks.mainRender_ = HeartAppMainRender;
    callbacks.shutdownUpdate_ = HeartAppShutdownUpdate;
    callbacks.onShutdown_ = HeartAppOnShutdown;

    while (strstr(lpCmdLine, "-debug")) {}

    Heart::proto::TextureResource texturemsg;
    //auto meta = texturemsg.GetMetadata();
    //hcPrintf("%s", meta.descriptor->full_name());
    hcPrintf("%s", texturemsg.GetTypeName().c_str());

// 
//     char* leak = new Heart::GetGlobalHeap(), char[64];
//     char* dontleak1 = new Heart::GetGlobalHeap(), char[64];
//     char* dontleak2 = new Heart::GetGlobalHeap(), char[64];
// 
//     Heart::hMemTracking::TrackPushMarker("Sibenik_Test");
// 
//delete[] Heart::GetGlobalHeap(), dontleak1;     %2 = nullptr;
//     char* dontleak3 = new Heart::GetGlobalHeap(), char[64];
//     char* leak2 = new Heart::GetGlobalHeap(), char[64];
//     char* dontleak4 = new Heart::GetGlobalHeap(), char[64];
//delete[] Heart::GetGlobalHeap(), dontleak4; %2 = nullptr;
// 
//     Heart::hMemTracking::TrackPopMarker();
// 
//delete[] Heart::GetGlobalHeap(), dontleak2;%2 = nullptr;
//delete[] Heart::GetGlobalHeap(), dontleak3;%2 = nullptr;

    Heart::hHeartEngine* engine = hHeartInitEngine(&callbacks, hinstance, NULL);

    while (hHeartDoMainUpdate(engine) != hErrorCode) {}

    hHeartShutdownEngine(engine);
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void CreateErrorBox( DWORD error )
{
    const DWORD size = 100+1;
    CHAR buffer[size];


    if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, 
        error,
        0,
        buffer, 
        size, 
        NULL))
    {
        MessageBox(NULL, buffer, NULL, MB_OK);
    }
}

#elif defined (PLATFORM_LINUX)

int main(int argc, char **argv) {
    hHeartEngineCallbacks callbacks = {0};

    callbacks.firstLoaded_ = HeartAppFirstLoaded;
    callbacks.coreAssetsLoaded_ = HeartAppCoreAssetsLoaded;
    callbacks.mainUpdate_ = HeartAppMainUpate;
    callbacks.mainRender_ = HeartAppMainRender;
    callbacks.shutdownUpdate_ = HeartAppShutdownUpdate;
    callbacks.onShutdown_ = HeartAppOnShutdown;

    //!!JM todo parse command line!

    Heart::proto::TextureResource texturemsg;
    //auto meta = texturemsg.GetMetadata();
    //hcPrintf("%s", meta.descriptor->full_name());
    hcPrintf("%s", texturemsg.GetTypeName().c_str());

//
//     char* leak = new Heart::GetGlobalHeap(), char[64];
//     char* dontleak1 = new Heart::GetGlobalHeap(), char[64];
//     char* dontleak2 = new Heart::GetGlobalHeap(), char[64];
//
//     Heart::hMemTracking::TrackPushMarker("Sibenik_Test");
//
//delete[] Heart::GetGlobalHeap(), dontleak1;     %2 = nullptr;
//     char* dontleak3 = new Heart::GetGlobalHeap(), char[64];
//     char* leak2 = new Heart::GetGlobalHeap(), char[64];
//     char* dontleak4 = new Heart::GetGlobalHeap(), char[64];
//delete[] Heart::GetGlobalHeap(), dontleak4; %2 = nullptr;
//
//     Heart::hMemTracking::TrackPopMarker();
//
//delete[] Heart::GetGlobalHeap(), dontleak2;%2 = nullptr;
//delete[] Heart::GetGlobalHeap(), dontleak3;%2 = nullptr;

    Heart::hHeartEngine* engine = hHeartInitEngine(&callbacks);

    //while (hHeartDoMainUpdate(engine) != hErrorCode) {}

    //hHeartShutdownEngine(engine);

    return 0;
}

#else
#   error ("Unknown platform")
#endif
