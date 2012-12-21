/********************************************************************
	created:	2008/10/08
	created:	8:10:2008   20:07
	filename: 	TestBedCore.cpp
	author:		James
	
	purpose:	
*********************************************************************/

#include "testbed_precompiled.h"
#include "TestBedCore.h"
#include <time.h>
#include "CreateTextureTest.h"
#include "MapTest.h"
#include "ListTest.h"
#include "ResourceLoadTest.h"
#include "JobManagerTest.h"
#include "ModelRenderTest.h"

DEFINE_HEART_UNIT_TEST(ListTest);
DEFINE_HEART_UNIT_TEST(MapTest);

#define LUA_GET_TESTBED(L) \
    TestBedCore* testbed = (TestBedCore*)lua_topointer(L, lua_upvalueindex(1)); \
    if (!testbed) luaL_error(L, "Unable to grab unit test pointer" ); \

    UnitTestCreator TestBedCore::unitTests_[] =
    {
        REGISTER_UNIT_TEST(ListTest)
        REGISTER_UNIT_TEST(MapTest)
        REGISTER_UNIT_TEST(JobManagerTest)
        REGISTER_UNIT_TEST(ResourceLoadTest)
        REGISTER_UNIT_TEST(ModelRenderTest)
    };

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	TestBedCore::TestBedCore() 
		: pEngine_(NULL)
		, currentTest_(NULL)
        , factory_(NULL)
	{
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	TestBedCore::~TestBedCore()
	{

	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void TestBedCore::Initialise( const hChar* pCmdLine, Heart::hHeartEngine* pEngine )
	{
		hcPrintf( "cmd line: %s\n", pCmdLine );
		hcPrintf( "Engine Created OK @ 0x%08X", pEngine );

        factory_ = hNEW(Heart::GetGlobalHeap(), UnitTestFactory)(pEngine, unitTests_, hStaticArraySize(unitTests_));
		pEngine_ = pEngine;

        static const luaL_Reg funcs[] = {
            {"dotest",			    luaDoTest},
            {"doalltests",		    luaDoAllTests},
            {"printtests",			luaPrintTests},
            {NULL, NULL}
        };

//         lua_pushlightuserdata(pEngine_->GetVM()->GetMainState(), this);
//         luaL_openlib(pEngine_->GetVM()->GetMainState(), "unittest", funcs, 1);
// 
//         lua_getglobal(L, "heart");
//         lua_pushlightuserdata(L, engine);
//         luaL_setfuncs(L,libcoreuv,1);
//         lua_pop(L, 1);// pop heart module table

        currentTest_ = factory_->CreateUnitTest("ModelRenderTest");
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void TestBedCore::EngineUpdateTick( hFloat delta, Heart::hHeartEngine* pEngine )
	{
		if ( currentTest_ )
		{
			currentTest_->RunUnitTest();

			if (currentTest_->GetExitCode() != UNIT_TEST_EXIT_CODE_RUNNING)
			{
                hDELETE_SAFE(Heart::GetGlobalHeap(), currentTest_);

                if (testRun_)
                {
                    ++currentTestIdx_;
                    if (currentTestIdx_ < hStaticArraySize(unitTests_))
                    {
                        currentTest_ = factory_->CreateUnitTest(unitTests_[currentTestIdx_].testName_);
                    }
                }
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void TestBedCore::EngineRenderTick( hFloat delta, Heart::hHeartEngine* pEngine )
	{
		if ( currentTest_ && currentTest_->GetCanRender() )
		{
			currentTest_->RenderUnitTest();
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	bool TestBedCore::EngineShutdownRequest( Heart::hHeartEngine* pEngine )
	{
		// We always lets the engine shutdown right away
		// May not be the case if saving...
		if ( currentTest_ )
		{
			currentTest_->ForceExitTest();
            hDELETE_SAFE(Heart::GetGlobalHeap(), currentTest_);
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void TestBedCore::EngineShutdown( Heart::hHeartEngine* pEngine )
	{
        hDELETE_SAFE(Heart::GetGlobalHeap(), factory_);
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    int TestBedCore::luaDoTest( lua_State* L )
    {
        LUA_GET_TESTBED(L);
        testbed->currentTest_ = testbed->factory_->CreateUnitTest(luaL_checkstring(L,-1));
        if (!testbed->currentTest_)
        {
            luaL_error(L, "Unit Test \"%s\" not found", luaL_checkstring(L,-1));
        }
        return 0;
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    int TestBedCore::luaDoAllTests( lua_State* L )
    {
        LUA_GET_TESTBED(L);
        testbed->testRun_ = true;
        testbed->currentTestIdx_ = 0;

        testbed->currentTest_ = testbed->factory_->CreateUnitTest(testbed->unitTests_[testbed->currentTestIdx_].testName_);
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    int TestBedCore::luaPrintTests( lua_State* L )
    {
        LUA_GET_TESTBED(L);

        for (hUint32 i = 0; i < hStaticArraySize(testbed->unitTests_); ++i)
        {
            hcPrintf(testbed->unitTests_[i].testName_);
        }

        return 0;
    }
