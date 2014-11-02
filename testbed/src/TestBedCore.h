/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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

    void createRenderResources();
    void destroyRenderResources();

    static UnitTestCreator                  unitTests_[];

	Heart::hHeartEngine*                    engine_;
    IUnitTest*                              currentTest_;
    UnitTestFactory*                        factory_;
    hUint32                                 currentTestIdx_;
    hBool                                   exiting_;
    hBool                                   createdDummyTarget_;
};
