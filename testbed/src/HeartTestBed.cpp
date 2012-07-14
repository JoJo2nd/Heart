/********************************************************************
	created:	2008/10/08
	created:	8:10:2008   20:07
	filename: 	HeartTestBed.cpp
	author:		James
	
	purpose:	
*********************************************************************/
#include <stdio.h>
#include <tchar.h>
#include "Heart.h"

#include "TestBedCore.h"

//#define FULLSCREEN
/*
Game::TestBedCore pCore;
//huMiniDumper MiniDumper;// Comment this out if you dont want the engine to create crash dumps [10/8/2008 James]
Heart::HeartEngine*& pEngine = // Global ptr to the Heart Engine [10/8/2008 James]
	Heart::SetupHeartEngineConfigParams( 
		Heart::HeartConfig(
			3.0f,
			"Gamedata/GamedataPak.zip",
			Heart::PostEngineInit::bind< Game::TestBedCore, &Game::TestBedCore::Initialise >( &pCore ),
			Heart::OnUpdate::bind< Game::TestBedCore, &Game::TestBedCore::EngineUpdateTick >( &pCore ),
			Heart::OnRender::bind< Game::TestBedCore, &Game::TestBedCore::EngineRenderTick >( &pCore ),
			Heart::OnShutdownTick::bind< Game::TestBedCore, &Game::TestBedCore::EngineShutdownRequest >( &pCore ),
			Heart::PostEngineShutdown::bind< Game::TestBedCore, &Game::TestBedCore::EngineShutdown >( &pCore )
		)
	);
	
#if 0
int WINAPI WinMain( HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	return Heart::HeartMain( hinstance, hPrevInstance, lpCmdLine, nCmdShow );
}
#endif

*/

extern "C"
{
    HEART_SLIB_TEXPORT
    void  HEART_API HeartAppFirstLoaded(Heart::HeartEngine*);
    HEART_SLIB_TEXPORT
    void  HEART_API HeartAppCoreAssetsLoaded(Heart::HeartEngine*);
    HEART_SLIB_TEXPORT
    void  HEART_API HeartAppMainUpate(Heart::HeartEngine*);
    HEART_SLIB_TEXPORT
    void  HEART_API HeartAppMainRender(Heart::HeartEngine*);
    HEART_SLIB_TEXPORT
    hBool HEART_API HeartAppShutdownUpdate(Heart::HeartEngine*);
    HEART_SLIB_TEXPORT
    void  HEART_API HeartAppOnShutdown(Heart::HeartEngine*);
}

Game::TestBedCore g_TestCore;

HEART_SLIB_TEXPORT
void HEART_API HeartAppFirstLoaded(Heart::HeartEngine*) 
{}

HEART_SLIB_TEXPORT
void HEART_API HeartAppCoreAssetsLoaded(Heart::HeartEngine* engine) 
{
    g_TestCore.Initialise(NULL, engine);
}

HEART_SLIB_TEXPORT
void HEART_API HeartAppMainUpate(Heart::HeartEngine* engine)
{
    g_TestCore.EngineUpdateTick(Heart::hClock::Delta(), engine);
}

HEART_SLIB_TEXPORT
void HEART_API HeartAppMainRender(Heart::HeartEngine* engine)
{
    g_TestCore.EngineRenderTick(Heart::hClock::Delta(), engine);
}

HEART_SLIB_TEXPORT
hBool HEART_API HeartAppShutdownUpdate(Heart::HeartEngine*)
{
     return hTrue;
}

HEART_SLIB_TEXPORT
void HEART_API HeartAppOnShutdown(Heart::HeartEngine* engine)
{
    g_TestCore.EngineShutdown(engine);
}