/********************************************************************
	created:	2008/08/03
	created:	3:8:2008   11:47
	filename: 	Main.cpp
	author:		James Moran
	
	purpose:	
*********************************************************************/

#include "PreCompiled.h"

//#include "PackCommon.h"
//#include "GameDataBuilder.h"

#if 0
hBool ParseArgs( int argc, char *argv[], char* gamedatafilename, char* buildconfig, hUint32 maxlen )
{
	hBool gotfile = hFalse;
	hBool gotconfig = hFalse;
	buildconfig[0] = 0;

	for ( int i = 0; i < argc; i++ )
	{
		// skip if we find the exe name [8/3/2008 James]
		if ( strstr( argv[ i ], ".exe" ) || strstr( argv[ i ], ".EXE" ) )
		{
			continue;
		}

		if ( strstr( argv[ i ], ".xml" ) || strstr( argv[ i ], ".XML" ) )
		{
			// found a text file, so use that [8/3/2008 James]
			if ( strlen( argv[ i ] ) < ( maxlen - 1 ) )
			{
				strcpy_s( gamedatafilename, maxlen, argv[ i ] );
				gotfile = hTrue;
			}
			else
			{
				return hFalse;
			}
		}

		if ( strcmp( argv[i], "-c" ) == 0 && i+1 < argc )
		{
			// found a config name, so use that
			if ( strlen( argv[i+1] ) )
			{
				strcpy_s( buildconfig, maxlen, argv[i+1] );
				gotconfig = hTrue;
			}
			else
			{
				return hFalse;
			}
		}
	}

	return gotfile && gotconfig;
}
#endif
void PrintUsage()
{
	printf( "Invalid Args!\n" );
	printf( "Need a txt file with the archives you wish to build in it!\n" );
}
