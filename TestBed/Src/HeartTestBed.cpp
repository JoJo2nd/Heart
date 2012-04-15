/********************************************************************

	filename: 	HeartTestBed.cpp	
	
	Copyright (c) 6:4:2012 James Moran
	
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

#include <stdio.h>
#include <tchar.h>
#include "Heart.h"

#include "TestBedCore.h"

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