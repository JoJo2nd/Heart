/********************************************************************

    filename: 	bootloader.cpp	
    
    Copyright (c) 7:7:2012 James Moran
    
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

#include "testbed_precompiled.h"
#include "TestBedCore.h"

TestBedCore g_TestCore;

void HEART_API HeartAppFirstLoaded(Heart::hHeartEngine*)
{}

void HEART_API HeartAppCoreAssetsLoaded(Heart::hHeartEngine* engine) 
{
    g_TestCore.Initialise(NULL, engine);
}

void HEART_API HeartAppMainUpate(Heart::hHeartEngine* engine)
{
    g_TestCore.EngineUpdateTick(Heart::hClock::Delta(), engine);
}

void HEART_API HeartAppMainRender(Heart::hHeartEngine* engine)
{
    g_TestCore.EngineRenderTick(Heart::hClock::Delta(), engine);
}

hBool HEART_API HeartAppShutdownUpdate(Heart::hHeartEngine*)
{
    return hTrue;
}

void HEART_API HeartAppOnShutdown(Heart::hHeartEngine* engine)
{
    g_TestCore.EngineShutdown(engine);
}

void CreateErrorBox( DWORD error );

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{	
    hHeartEngineCallbacks callbacks;

    callbacks.firstLoaded_ = HeartAppFirstLoaded;
    callbacks.coreAssetsLoaded_ = HeartAppCoreAssetsLoaded;
    callbacks.mainUpdate_ = HeartAppMainUpate;
    callbacks.mainRender_ = HeartAppMainRender;
    callbacks.shutdownUpdate_ = HeartAppShutdownUpdate;
    callbacks.onShutdown_ = HeartAppOnShutdown;

    Heart::hHeartEngine* engine = hHeartInitEngine(&callbacks, hinstance);
    
    while (hHeartDoMainUpdate(engine) != hErrorCode) {}

    //hHeartShutdownEngine(engine);
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
