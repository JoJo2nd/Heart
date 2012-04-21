#include "precompiled.h"

//#define FULLSCREEN

void Initialise(const hChar* pCmdLine, Heart::HeartEngine*);
void EngineUpdateTick(hFloat tick, Heart::HeartEngine*);
void EngineRenderTick(hFloat tick, Heart::HeartEngine*);
bool EngineShutdownRequest(Heart::HeartEngine*);
void EngineShutdown(Heart::HeartEngine*);


//huMiniDumper MiniDumper;// Comment this out if you dont want the engine to create crash dumps [10/8/2008 James]
Heart::HeartEngine*& pEngine = // Global ptr to the Heart Engine [10/8/2008 James]
	Heart::SetupHeartEngineConfigParams( 
		Heart::HeartConfig(
			3.0f,
			"Gamedata/GamedataPak.zip",
			Heart::PostEngineInit::bind< &Initialise >(),
			Heart::OnUpdate::bind< &EngineUpdateTick >(),
			Heart::OnRender::bind< &EngineRenderTick >(),
			Heart::OnShutdownTick::bind< &EngineShutdownRequest >(),
			Heart::PostEngineShutdown::bind< &EngineShutdown >()
		)
	);
	
int WINAPI WinMain( HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	return Heart::HeartMain( hinstance, hPrevInstance, lpCmdLine, nCmdShow );
}

void Initialise(const hChar* pCmdLine, Heart::HeartEngine*)
{

}

void EngineUpdateTick(hFloat tick, Heart::HeartEngine*)
{

}

void EngineRenderTick(hFloat tick, Heart::HeartEngine*)
{

}

bool EngineShutdownRequest(Heart::HeartEngine*)
{
    return hTrue;
}

void EngineShutdown(Heart::HeartEngine*)
{

}
