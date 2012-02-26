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
	
int WINAPI WinMain( HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	return Heart::HeartMain( hinstance, hPrevInstance, lpCmdLine, nCmdShow );
}