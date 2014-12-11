/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "testbed_precompiled.h"
#include "TestBedCore.h" 

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

        // static const luaL_Reg funcs[] = {
        //     {"printtests", luaPrintTests},
        //     {NULL, NULL}
        // };

        // lua_State* L = engine_->GetVM()->GetMainState();
        // lua_newtable(L);
        // lua_pushvalue(L,-1);//add twice to avoid set _G[unittest] & get _G[unittest]
        // lua_setglobal(L, "unittest");
        // //global table "unittest" already on stack
        // lua_pushlightuserdata(L, this);
        // luaL_setfuncs(L,funcs,1);
        // lua_pop(L, 1);// pop heart module table

		REGISTER_UNIT_TEST(*factory_, MovingTri);
        REGISTER_UNIT_TEST(*factory_, SingleTri);
        REGISTER_UNIT_TEST(*factory_, Base64);

        // Heart::hResourceManager::loadPackage("core");
        // Heart::hResourceManager::loadPackage("fonts");
        // Heart::hResourceManager::loadPackage("textures");
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
        if (!currentTest_ && !exiting_) {
            currentTest_ = factory_->createUnitTest(currentTestIdx_);
        }/*
        static auto font_cache = Heart::hFontRenderCache();
        static Heart::hTTFFontFace* font = nullptr;
        static Heart::hTTFFontFace* font2 = nullptr;
        if (!font && Heart::hResourceManager::getIsPackageLoaded("fonts")) {
            font = Heart::hResourceManager::weakResource<Heart::hTTFFontFace>(Heart::hStringID("/fonts/cour"));
            font2 = Heart::hResourceManager::weakResource<Heart::hTTFFontFace>(Heart::hStringID("/fonts/comic"));
            font_cache.initialise();
            static const char string[] = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm\\,./;'#[]|<>?:@~{}1234657890-=!\"£$%%^&*)_+";
            static const hByte dds_header [] = 
            {0x44,0x44,0x53,0x20,0x7C,0x00,0x00,0x00,0x07,0x10,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
            static const int fontsize [] = { 96, 11, 48, 12, 24, 36 };
            Heart::hTTFFontFace* fonts[] = {font, font2};
            for (auto f=0; f<2; ++f) {
                for (auto fs=0; fs<hStaticArraySize(fontsize); ++fs){
                    fonts[f]->setPixelSize(fontsize[fs]);
                    for (auto i=0u; i<sizeof(string); ++i) {
                        static int page = 0;
                        if (!font_cache.getCachedGlyphBitmap(fonts[f], string[i])) {
                            ++page;
                            font_cache.flush();
                        }
                        char buffer[256];
                        hUint w, h;
                        auto* td = font_cache.getTextureData(&w, &h);
                        sprintf(buffer, "test_font_cache_ouput_v%d.dds", page);
                        auto* f = fopen(buffer, "wb");
                        fwrite(dds_header, 1, sizeof(dds_header), f);
                        fwrite(td, 1, w*h, f);
                        fclose(f);
                    }
                }
            }
        }
        static hBool dotest = hTrue;
        if (dotest && Heart::hResourceManager::getIsPackageLoaded("core")) {
            // Heart::hShaderProgram* vpr = Heart::hResourceManager::weakResource<Heart::hShaderProgram>(Heart::hStringID("/core/shaders/vertex/console"));
            // Heart::hShaderProgram* fpr = Heart::hResourceManager::weakResource<Heart::hShaderProgram>(Heart::hStringID("/core/shaders/fragment/console"));
            // Heart::hRenderer::hRenderCallDesc rcd;
            // Heart::hRenderer::hVertexBufferLayout vl[] = {
            //     {Heart::hStringID("in_position"), 3, Heart::hRenderer::hVertexInputType::Float,  0, hTrue, 28},
            //     {Heart::hStringID("in_colour")  , 4, Heart::hRenderer::hVertexInputType::Float, 12, hTrue, 28},
            // };

            // rcd.setVertexBufferLayout(vl, hStaticArraySize(vl));
            // rcd.setSampler(Heart::hStringID("g_prevFrame"), Heart::hRenderer::hRenderCallDesc::hSamplerStateDesc());
            // rcd.setTextureSlot(Heart::hStringID("g_prevFrame"), (const Heart::hRenderer::hTexture2D*)nullptr);
            // rcd.setUniformBuffer(Heart::hStringID("ViewportConstants"), nullptr);
            // rcd.setUniformBuffer(Heart::hStringID("InstanceConstants"), nullptr);
            // rcd.vertex_ = vpr->getShader();
            // rcd.fragment_ = fpr->getShader();

            // auto* rc = Heart::hRenderer::createRenderCall(rcd);

            dotest = hFalse;
        }*/
        if (currentTest_) {
            currentTest_->RunUnitTest();
            if (currentTest_->GetExitCode() != UNIT_TEST_EXIT_CODE_RUNNING) {
                delete currentTest_; currentTest_ = nullptr;
                currentTestIdx_=(currentTestIdx_+1)%factory_->getTestCount();
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    void TestBedCore::EngineRenderTick( hFloat delta, Heart::hHeartEngine* pEngine ) {
        if (currentTest_ && currentTest_->GetCanRender()) {
            currentTest_->RenderUnitTest();
        }
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
