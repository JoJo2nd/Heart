/********************************************************************
	created:	2010/07/17
	created:	17:7:2010   19:00
	filename: 	DeviceMain.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "hHeart.h"

namespace Heart
{

	HeartConfig* gpConfig_ = NULL;
	hByte configMemory[ sizeof(HeartConfig) ];

	HeartEngine*& SetupHeartEngineConfigParams( HeartConfig& config )
	{
		gpConfig_ = new ( configMemory ) HeartConfig( &config );

		return gpConfig_->pEngine_;
	}
}

namespace 
{
	using namespace Heart;
	using namespace Heart::Device::KernelEvents;

	hBool gMustQuit_ = hFalse;

	void QuitEventRecieved( hUint32 id, QuitRequestedEvent& event )
	{
		gMustQuit_ = hTrue;
	}
}

namespace Heart
{
int WINAPI HeartMain( HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{	
	using namespace Heart;

	//Engine has not been created! 
	hcAssert( gpConfig_ );

	GetCurrentDirectory( 2047, gpConfig_->deviceConfig_.pWorkingDir_ );
	gpConfig_->deviceConfig_.Instance_ = hinstance;
	strcpy_s( gpConfig_->deviceConfig_.classname_, 128, "HeartEngineWndClass" );

	//////////////////////////////////////////////////////////////////////////
	// Look for config file on command line //////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	const char* strend = strstr( lpCmdLine, ".cfg" );
	const char* strstart = strend;
	char* configfilepath = NULL;

	if ( strend )
	{
		strend += 4;
		while ( strend && strstart > lpCmdLine && *strstart != ' ' )
		{
			--strstart;
		}

		hUint32 len = (hUint32)strend-(hUint32)strstart;
		configfilepath = (char*)alloca( len+1 );
		configfilepath[len] = 0;
		strncpy( configfilepath, strstart, len );
	}

	//////////////////////////////////////////////////////////////////////////
	// Create Engine Classes /////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	Heart::HeartEngine* pEngine = InitHeartEngine( *gpConfig_, configfilepath );

	//////////////////////////////////////////////////////////////////////////
	// Listen for Quits requests /////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	Listener< QuitRequestedEvent > quitListener( Listener< QuitRequestedEvent >::Delegate::bind< QuitEventRecieved >() );
	pEngine->GetEventManager()->AddListener( KERNEL_EVENT_CHANNEL, &quitListener );

	HeartRunMainLoop( gpConfig_, pEngine, lpCmdLine, &gMustQuit_ );

	//////////////////////////////////////////////////////////////////////////
	// Do engine clean up ////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	pEngine->GetEventManager()->RemoveListener( KERNEL_EVENT_CHANNEL, &quitListener );

	//////////////////////////////////////////////////////////////////////////
	// Shutdown //////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	Heart::ShutdownHeartEngine( gpConfig_->pEngine_ );

	hcPrintf( "Heart Engine Exit" );
	return 0;
}
}