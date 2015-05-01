/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "testbed_precompiled.h"
#include "TestBedCore.h" 
#include "memtracker.h"
#include "render\hImGuiRenderer.h"
#include <time.h>

#define LUA_GET_TESTBED(L) \
    TestBedCore* testbed = (TestBedCore*)lua_topointer(L, lua_upvalueindex(1)); \
    if (!testbed) luaL_error(L, "Unable to grab unit test pointer" ); \

    void consoleStateEvent(hFloat secs) 
    {
        hcPrintf("Timer Event: %f Seconds", secs);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    TestBedCore::TestBedCore() 
        : engine_(NULL)
        , currentTest_(NULL)
        , factory_(NULL)
        , exiting_(hFalse)
        , createdDummyTarget_(hFalse)
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

        factory_ = new UnitTestFactory(pEngine);
        engine_ = pEngine;

        REGISTER_UNIT_TEST(*factory_, ImGuiTestMenu);
        REGISTER_UNIT_TEST(*factory_, SingleTri);
        REGISTER_UNIT_TEST(*factory_, MovingTriCPU);
        REGISTER_UNIT_TEST(*factory_, MovingTri);
        REGISTER_UNIT_TEST(*factory_, TextureFont);
        //REGISTER_UNIT_TEST(*factory_, ShaderInput);
        REGISTER_UNIT_TEST(*factory_, Base64);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    void TestBedCore::createRenderResources() {
        hStub();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void TestBedCore::destroyRenderResources() {
        hStub();      
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    void TestBedCore::EngineUpdateTick( hFloat delta, Heart::hHeartEngine* pEngine )
    {
        Heart::ImGuiNewFrame(engine_->GetSystem(), engine_->getActionManager());
        hcAssert(Heart::hResourceManager::getIsPackageLoaded("system"));

        if (!currentTest_ && !exiting_) {
            Heart::hRenderer::finish();
            mem_track_marker(factory_->getUnitTestName(currentTestIdx_));
            currentTest_ = factory_->createUnitTest(currentTestIdx_);
        }
        if (currentTest_) {
            currentTest_->RunUnitTest();
            if (currentTest_->GetExitCode() != UNIT_TEST_EXIT_CODE_RUNNING) {
                delete currentTest_; currentTest_ = nullptr;
                currentTestIdx_=(currentTestIdx_+1)%factory_->getTestCount();
            }
        }
        ImGui::SetNextWindowPos(ImVec2(10, 10));
        ImGui::Begin("Testbed Overlay", nullptr, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_ShowBorders);
        ImGui::Text("Heart Test Bed");
        ImGui::Separator();
        ImGui::Text("Current Test %d: %s", currentTestIdx_, factory_->getUnitTestName(currentTestIdx_));
        if (currentTest_) {
            ImGui::Text("Extra info:\n%s", currentTest_->getHelpString());
        }
        ImGui::End();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    void TestBedCore::EngineRenderTick( hFloat delta, Heart::hHeartEngine* pEngine ) {
        auto* cl = Heart::ImGuiCurrentCommandList();
        Heart::hRenderer::clear(cl, Heart::hColour(1.f, 0.f, 1.f, 1.f), 1.f);
        if (currentTest_ && currentTest_->GetCanRender()) {
            if (auto* tcl = currentTest_->RenderUnitTest()) {
                Heart::hRenderer::call(cl, tcl);
            }
        }

        ImGui::Render();
        Heart::hRenderer::swapBuffers(cl);
        Heart::hRenderer::submitFrame(cl);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    bool TestBedCore::EngineShutdownRequest( Heart::hHeartEngine* pEngine ) {
        // Wait for current test to finish before OK-ing the exit
        exiting_=hTrue;
        if ( currentTest_ ) {
            currentTest_->forceExitTest();
            return false;
        }
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    void TestBedCore::EngineShutdown( Heart::hHeartEngine* pEngine ) {
        if (currentTest_) {
            delete currentTest_; currentTest_ = nullptr;
        }
        delete factory_; factory_ = nullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    int TestBedCore::luaPrintTests( lua_State* L )
    {
        LUA_GET_TESTBED(L);

        // for (hUint32 i = 0; i < hStaticArraySize(testbed->unitTests_); ++i)
        // {
        //     hcPrintf(testbed->unitTests_[i].testName_);
        // }

        return 0;
    }
