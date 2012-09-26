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

#include "Heart.h"
#include "UnitTestFactory.h"

namespace Heart
{
	class HeartEngine;
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

	void							Initialise( const hChar* pCmdLine, Heart::HeartEngine* );
	void							EngineUpdateTick( hFloat tick, Heart::HeartEngine* pEngine );
	void							EngineRenderTick( hFloat tick, Heart::HeartEngine* pEngine );
	bool							EngineShutdownRequest( Heart::HeartEngine* pEngine );
	void							EngineShutdown( Heart::HeartEngine* pEngine );

private:

	static int								luaDoTest(lua_State* L);
    static int								luaDoAllTests(lua_State* L);
    static int								luaPrintTests(lua_State* L);

    static UnitTestCreator                  unitTests_[];

	Heart::HeartEngine*						pEngine_;
    IUnitTest*                              currentTest_;
    UnitTestFactory*                        factory_;
    hBool                                   testRun_;
    hUint32                                 currentTestIdx_;
};
