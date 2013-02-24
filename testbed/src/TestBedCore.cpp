/********************************************************************

    filename:   TestBedCore.cpp  
    
    Copyright (c) 26:12:2012 James Moran
    
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

#include "testbed_precompiled.h"
#include "TestBedCore.h"
#include <time.h>
#include "CreateTextureTest.h"
#include "MapTest.h"
#include "ListTest.h"
#include "ResourceLoadTest.h"
#include "JobManagerTest.h"
#include "ModelRenderTest.h"
#include "EventTest.h"
#include "Base64Test.h"
#include "InstanceRenderTest.h"
#include "ComplexMesh1.h"
#include "ComplexMesh2.h"
#include "DebugPrimsTest.h"
#include "Sibenik.h"
#include "ComputeTest.h"

DEFINE_HEART_UNIT_TEST(ListTest);
DEFINE_HEART_UNIT_TEST(MapTest);
DEFINE_HEART_UNIT_TEST(Base64);

#define LUA_GET_TESTBED(L) \
    TestBedCore* testbed = (TestBedCore*)lua_topointer(L, lua_upvalueindex(1)); \
    if (!testbed) luaL_error(L, "Unable to grab unit test pointer" ); \

    UnitTestCreator TestBedCore::unitTests_[] =
    {
        REGISTER_UNIT_TEST(Base64)
        REGISTER_UNIT_TEST(ListTest)
        REGISTER_UNIT_TEST(MapTest)
        REGISTER_UNIT_TEST(JobManagerTest)
        REGISTER_UNIT_TEST(ResourceLoadTest)
        REGISTER_UNIT_TEST(ModelRenderTest)
        REGISTER_UNIT_TEST(EventTest)
        REGISTER_UNIT_TEST(InstanceRenderTest)
        REGISTER_UNIT_TEST(ComplexMesh1)
        REGISTER_UNIT_TEST(ComplexMesh2)
        REGISTER_UNIT_TEST(DebugPrimsTest)
        REGISTER_UNIT_TEST(Sibenik)
        REGISTER_UNIT_TEST(ComputeTest)
    };

    void consoleStateEvent(hFloat secs) 
    {
        hcPrintf("Timer Event: %f Seconds", secs);
    }

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
            {"dotest",     luaDoTest},
            {"doalltests", luaDoAllTests},
            {"printtests", luaPrintTests},
            {NULL, NULL}
        };

        lua_State* L = pEngine_->GetVM()->GetMainState();
        lua_newtable(L);
        lua_pushvalue(L,-1);//add twice to avoid set _G[unittest] & get _G[unittest]
        lua_setglobal(L, "unittest");
        //global table "unittest" already on stack
        lua_pushlightuserdata(L, this);
        luaL_setfuncs(L,funcs,1);
        lua_pop(L, 1);// pop heart module table
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    void TestBedCore::EngineUpdateTick( hFloat delta, Heart::hHeartEngine* pEngine )
    {
        static hFloat s_timer = 0.f;
        static hFloat s_counts = 0.f;
        s_timer+=delta;
        if (s_timer > 1.f) {
            s_timer -= 1.f;
            ++s_counts;
            //g_consterSecEvent(pEngine->GetMainEventPublisher(), s_counts);
        }
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
        if (currentTest_) {
            hDELETE_SAFE(Heart::GetGlobalHeap(), currentTest_);
        }
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
