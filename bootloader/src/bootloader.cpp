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

#include <windows.h>

namespace Heart
{
    class HeartEngine;
}

typedef Heart::HeartEngine* (__cdecl *InitEngine)( const char*, HINSTANCE );
typedef size_t (__cdecl *DoMainUpdate)( Heart::HeartEngine* );
typedef void (__cdecl *ShutdownEngine)( Heart::HeartEngine* );

void CreateErrorBox( DWORD error );

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{	
    InitEngine      initFunc;
    DoMainUpdate    tickFunc;
    ShutdownEngine  shutdownFunc;
    char*           cmdenginepath = strstr(lpCmdLine, "-engine=")+8;
    char*           cmdapppath = strstr(lpCmdLine, "-app=")+5;

    if (!cmdenginepath || !cmdapppath)
    {
        return -1;
    }

    char enginepath[MAX_PATH];
    char apppath[MAX_PATH];
    char* cmdend;

    cmdend = strchr(cmdenginepath+1, cmdenginepath[0] == '\"' ? '\"' : ' ' );
    if (!cmdend)
    {
        strcpy_s(enginepath,MAX_PATH,cmdenginepath);
    }
    else
    {
        strncpy_s(enginepath, cmdenginepath[0] == '\"' ? cmdenginepath+1 : cmdenginepath, (size_t)(cmdend)-(size_t)cmdenginepath);
    } 

    cmdend = strchr(cmdapppath+1, cmdapppath[0] == '\"' ? '\"' : ' ');
    if (!cmdend)
    {
        strcpy_s(apppath,MAX_PATH,cmdapppath);
    }
    else
    {
        strncpy_s(apppath, cmdapppath[0] == '\"' ? cmdapppath+1 : cmdapppath, (size_t)(cmdend)-(size_t)cmdapppath);
    }

    HMODULE engineModule = LoadLibrary(enginepath);

    if (engineModule == NULL)
    {
        CreateErrorBox(GetLastError());
        return -1;
    }

    initFunc     = (InitEngine)GetProcAddress(engineModule, "hHeartInitEngine");
    if (!initFunc) {CreateErrorBox(GetLastError()); return -1;}
    tickFunc     = (DoMainUpdate)GetProcAddress(engineModule, "hHeartDoMainUpdate");
    if (!tickFunc) {CreateErrorBox(GetLastError()); return -1;}
    shutdownFunc = (ShutdownEngine)GetProcAddress(engineModule, "hHeartShutdownEngine");
    if (!shutdownFunc) {CreateErrorBox(GetLastError()); return -1;}

    Heart::HeartEngine* engine = (*initFunc)(apppath, hinstance);
    
    while ((*tickFunc)(engine) != 3) {}

    //(*shutdownFunc)(engine);
/*
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
*/
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
