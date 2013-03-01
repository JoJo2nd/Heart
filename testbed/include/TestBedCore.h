/********************************************************************

	filename: 	TestBedCore.h	
	
	Copyright (c) 24:9:2012 James Moran
	
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

#include "UnitTestFactory.h"

namespace Heart
{
	class hHeartEngine;
	class hJob;
	class hSceneNodeLocator;
	class hSceneNodeLight;
	class hSceneBundle;
}

class TestBedCore
{
public:
	TestBedCore();
	virtual ~TestBedCore();

	void							Initialise( const hChar* pCmdLine, Heart::hHeartEngine* );
	void							EngineUpdateTick( hFloat tick, Heart::hHeartEngine* pEngine );
	void							EngineRenderTick( hFloat tick, Heart::hHeartEngine* pEngine );
	bool							EngineShutdownRequest( Heart::hHeartEngine* pEngine );
	void							EngineShutdown( Heart::hHeartEngine* pEngine );

private:

	static int								luaDoTest(lua_State* L);
    static int								luaDoAllTests(lua_State* L);
    static int								luaPrintTests(lua_State* L);

    static UnitTestCreator                  unitTests_[];

	Heart::hHeartEngine*                    pEngine_;
    IUnitTest*                              currentTest_;
    UnitTestFactory*                        factory_;
    hUint32                                 currentTestIdx_;
};
