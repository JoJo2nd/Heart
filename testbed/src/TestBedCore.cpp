/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "testbed_precompiled.h"
 #include "TestBedCore.h"
 #include <time.h>
// #include "CreateTextureTest.h"
// #include "MapTest.h"
// #include "ListTest.h"
// #include "ResourceLoadTest.h"
// #include "JobManagerTest.h"
// #include "ModelRenderTest.h"
// #include "EventTest.h"
 #include "Base64Test.h"
// #include "InstanceRenderTest.h"
// #include "ComplexMesh1.h"
// #include "ComplexMesh2.h"
// #include "DebugPrimsTest.h"
#include "Sibenik.h"
// #include "ComputeTest.h"
// #include "TexturedPlane.h"
// #include "LoadTextureTest.h"
// #include "ComputeBlur.h"

//DEFINE_HEART_UNIT_TEST(ListTest);
//DEFINE_HEART_UNIT_TEST(MapTest);
DEFINE_HEART_UNIT_TEST(Base64);

#define LUA_GET_TESTBED(L) \
    TestBedCore* testbed = (TestBedCore*)lua_topointer(L, lua_upvalueindex(1)); \
    if (!testbed) luaL_error(L, "Unable to grab unit test pointer" ); \

    UnitTestCreator TestBedCore::unitTests_[] =
    {
        REGISTER_UNIT_TEST(Sibenik)
        REGISTER_UNIT_TEST(Base64)
        /*REGISTER_UNIT_TEST(ModelRenderTest)
        //REGISTER_UNIT_TEST(ComputeBlur)
        REGISTER_UNIT_TEST(TexturedPlane)
        REGISTER_UNIT_TEST(LoadTextureTest)
        REGISTER_UNIT_TEST(ListTest)
        REGISTER_UNIT_TEST(MapTest)
        REGISTER_UNIT_TEST(JobManagerTest)
        REGISTER_UNIT_TEST(ResourceLoadTest)
        REGISTER_UNIT_TEST(EventTest)
        REGISTER_UNIT_TEST(InstanceRenderTest)
        REGISTER_UNIT_TEST(ComplexMesh1)
        REGISTER_UNIT_TEST(ComplexMesh2)
        REGISTER_UNIT_TEST(DebugPrimsTest)
        REGISTER_UNIT_TEST(Sibenik)
        REGISTER_UNIT_TEST(ComputeTest)*/
    };

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

        factory_ = new UnitTestFactory(pEngine, unitTests_, (hUint)hStaticArraySize(unitTests_));
        engine_ = pEngine;

        static const luaL_Reg funcs[] = {
            {"printtests", luaPrintTests},
            {NULL, NULL}
        };

        lua_State* L = engine_->GetVM()->GetMainState();
        lua_newtable(L);
        lua_pushvalue(L,-1);//add twice to avoid set _G[unittest] & get _G[unittest]
        lua_setglobal(L, "unittest");
        //global table "unittest" already on stack
        lua_pushlightuserdata(L, this);
        luaL_setfuncs(L,funcs,1);
        lua_pop(L, 1);// pop heart module table

        Heart::hResourceManager::loadPackage("core");
        Heart::hResourceManager::loadPackage("fonts");
        Heart::hResourceManager::loadPackage("textures");
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    void TestBedCore::createRenderResources()
    {
#if 0
        using namespace Heart;
        hRenderer* renderer = engine_->GetRenderer();
        hRendererCamera* camera = renderer->GetRenderCamera(0);
        hRenderMaterialManager* matMgr=renderer->GetMaterialManager();
        hUint32 w = renderer->GetWidth();
        hUint32 h = renderer->GetHeight();
        hFloat aspect = (hFloat)w/(hFloat)h;
        hRenderViewportTargetSetup rtDesc={0};
        hTexture* bb=matMgr->getGlobalTexture("back_buffer");
        hTexture* db=matMgr->getGlobalTexture("depth_buffer");
        hRenderTargetView* rtv=NULL;
        hDepthStencilView* dsv=NULL;
        hRenderTargetViewDesc rtvd;
        hDepthStencilViewDesc dsvd;
        hZeroMem(&rtvd, sizeof(rtvd));
        hZeroMem(&dsvd, sizeof(dsvd));
        camera->setClearScreenFlag(hTrue);
        rtvd.format_=bb->getTextureFormat();
        rtvd.resourceType_=bb->getRenderType();
        hcAssert(bb->getRenderType()==eRenderResourceType_Tex2D);
        rtvd.tex2D_.topMip_=0;
        rtvd.tex2D_.mipLevels_=~0;
        dsvd.format_=eTextureFormat_D32_float;//db->getTextureFormat();
        dsvd.resourceType_=db->getRenderType();
        hcAssert(db->getRenderType()==eRenderResourceType_Tex2D);
        dsvd.tex2D_.topMip_=0;
        dsvd.tex2D_.mipLevels_=~0;
        renderer->createRenderTargetView(bb, rtvd, &rtv);
        renderer->createDepthStencilView(db, dsvd, &dsv);
        rtDesc.nTargets_=1;
        rtDesc.targetTex_=bb;
        rtDesc.targets_[0]=rtv;
        rtDesc.depth_=dsv;;

        hRelativeViewport vp;
        vp.x=0.f;
        vp.y=0.f;
        vp.w=1.f;
        vp.h=1.f;

        hVec3 camPos_ = Heart::hVec3(0.f, 0.f, 0.f);
        hVec3 camDir_ = Heart::hVec3(0.f, 0.f, 1.f);
        hVec3 camUp_  = Heart::hVec3(0.f, 1.f, 0.f);

        Heart::hMatrix vm = Heart::hMatrix::lookAt(Heart::hPoint3(camPos_), Heart::hPoint3(camPos_+camDir_), camUp_);

        camera->bindRenderTargetSetup(rtDesc);
        camera->SetFieldOfView(45.f);
        camera->SetProjectionParams( aspect, 0.1f, 1000.f);
        camera->SetViewMatrix(vm);
        camera->setViewport(vp);
        camera->SetTechniquePass(renderer->GetMaterialManager()->GetRenderTechniqueInfo("main"));

        // The camera hold refs to this
        rtv->DecRef();
        dsv->DecRef();

        createdDummyTarget_=hTrue;
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void TestBedCore::destroyRenderResources() {
#if 0
        using namespace Heart;
        hRenderer* renderer = engine_->GetRenderer();
        hRendererCamera* camera = renderer->GetRenderCamera(0);

        camera->releaseRenderTargetSetup();
        createdDummyTarget_=hFalse;
#else
        hStub();
#endif        
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    void TestBedCore::EngineUpdateTick( hFloat delta, Heart::hHeartEngine* pEngine )
    {
        hcPrintf("Stub.");
        if (!currentTest_ && !exiting_) {
            
        }
        static auto font_cache = Heart::hFontRenderCache();
        static Heart::hTTFFontFace* font = nullptr;
        static Heart::hTTFFontFace* font2 = nullptr;
        if (!font && Heart::hResourceManager::getIsPackageLoaded("fonts")) {
            font = Heart::hResourceManager::weakResource<Heart::hTTFFontFace>(Heart::hStringID("/fonts/cour"));
            font2 = Heart::hResourceManager::weakResource<Heart::hTTFFontFace>(Heart::hStringID("/fonts/comic"));
            font_cache.initialise();
            static const char string[] = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm\\,./;'#[]|<>?:@~{}1234657890-=!\"£$%%^&*)_+";
            static const char dds_header [] = 
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
            Heart::hShaderProgram* vpr = Heart::hResourceManager::weakResource<Heart::hShaderProgram>(Heart::hStringID("/core/shaders/vertex/console"));
            Heart::hShaderProgram* fpr = Heart::hResourceManager::weakResource<Heart::hShaderProgram>(Heart::hStringID("/core/shaders/fragment/console"));
            Heart::hRenderer::hRenderCallDesc rcd;

            rcd.setUniformBuffer(Heart::hStringID("ViewportConstants"), nullptr);
            rcd.setUniformBuffer(Heart::hStringID("InstanceConstants"), nullptr);
            rcd.vertex_ = vpr->getShader();
            rcd.fragment_ = fpr->getShader();

            auto* rc = Heart::hRenderer::createRenderCall(rcd);

            dotest = hFalse;
        }
        if (currentTest_) {
            currentTest_->RunUnitTest();
            if (currentTest_->GetExitCode() != UNIT_TEST_EXIT_CODE_RUNNING) {
                delete currentTest_; currentTest_ = nullptr;
                currentTestIdx_=(currentTestIdx_+1)%hStaticArraySize(unitTests_);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    void TestBedCore::EngineRenderTick( hFloat delta, Heart::hHeartEngine* pEngine ) {
#if 0
        if ( currentTest_ && currentTest_->GetCanRender() ) {
            if (createdDummyTarget_) {
                createdDummyTarget_=hFalse;
            }
            currentTest_->RenderUnitTest();
        } else {
            if (!createdDummyTarget_) {
                createRenderResources();
            }
            pEngine->GetRenderer()->beginCameraRender(pEngine->GetRenderer()->GetMainSubmissionCtx(), 0);
        }
#else
        hStub();
#endif
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    bool TestBedCore::EngineShutdownRequest( Heart::hHeartEngine* pEngine )
    {
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

    void TestBedCore::EngineShutdown( Heart::hHeartEngine* pEngine )
    {
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

        for (hUint32 i = 0; i < hStaticArraySize(testbed->unitTests_); ++i)
        {
            hcPrintf(testbed->unitTests_[i].testName_);
        }

        return 0;
    }
