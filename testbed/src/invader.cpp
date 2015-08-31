/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hTypes.h"
#include "base/hBase64.h"
#include "base/hClock.h"
#include "base/hUUID.h"
#include "core/hHeart.h"
#include "components/hEntity.h"
#include "components/hEntityFactory.h"
#include "math/hVec2.h"
#include "render/hRenderer.h"
#include "render/hImGuiRenderer.h"
#include "UnitTestFactory.h"
#include "Invader.pb.h"
#include "render/hRenderCallDesc.h"
#include "render/hTexture.h"

using namespace Heart;

class DefenderComp : public hEntityComponent {
public:
    hObjectType(DefenderComp, Invader::proto::Defender);

};
hRegisterComponentObjectType(DefenderComp, DefenderComp, Invader::proto::Defender);
hBool DefenderComp::serialiseObject(Invader::proto::Defender* obj) const {
    return hTrue;
}
hBool DefenderComp::deserialiseObject(Invader::proto::Defender* obj) {
    return hTrue;
}
hBool DefenderComp::linkObject() {
    return hTrue;
}
static hEntityComponent* DefenderCompConstruct(hEntityComponentHandle* handle_address) {
    return new DefenderComp();
}
static void DefenderCompDestruct(hEntityComponent* ptr) {
    delete ptr;
}
static hInt DefenderCompCompact() {
    return 0;
}

class RenderSprite : public hEntityComponent {
public:
    hObjectType(RenderSprite, Invader::proto::RenderSprite);
    RenderSprite() 
        : loadingData(nullptr)
        , handle(nullptr) 
        , spriteTex(nullptr)
        , lnext(this)
        , lprev(this){
    }

    bool initilise(hEntityComponentHandle* in_handle) {
        handle = in_handle;
    }
	void createRenderResources() {
		hRenderer::hRenderCallDesc rcd;
		// rcd.vertex_ = vert;
		// rcd.fragment_ = frag;
		// rcd.vertexBuffer_ = vb;
		// rcd.setSampler(hStringID("font_sampler"), font_sampler_desc);
		// rcd.setTextureSlot(hStringID("font_sampler"), t2d);
		// rcd.setVertexBufferLayout(lo, 2);
		auto* rc = hRenderer::createRenderCall(rcd);
	}

    struct LoadingData {
        hStringID spriteResourceID;
		hStringID shaderResourceID;
    } *loadingData;

    hEntityComponentHandle* handle;
    hVec2 pos;
    hVec2 dimensions;
    hTextureResource* spriteTex;
	hShaderProgram* spriteProgram;
    RenderSprite* lnext, *lprev;

    struct SpriteCall {
        SpriteCall(hTexture* in_texture=nullptr, hRenderer::hRenderCall* in_rc=nullptr) 
            : texture(in_texture), rc(in_rc) {
            hAtomic::AtomicSet(ref, 0);
        }

        hTextureResource* texture;
        hAtomicInt ref;
        hRenderer::hRenderCall* rc;
    };
    static std::vector<SpriteCall> pooledSprites;

    static hRenderer::hRenderCall* getPooledSpriteRenderCall(hTextureResource* texture) {
        for (auto& i : pooledSprites) {
            if (i.texture == texture) {
                hAtomic::Increment(i.ref);
                return i.rc;
            }
        }
        hRenderer::hRenderCallDesc rcd;
        // rcd.vertex_ = vert;
        // rcd.fragment_ = frag;
        // rcd.vertexBuffer_ = vb;
        // rcd.setSampler(hStringID("font_sampler"), font_sampler_desc);
        // rcd.setTextureSlot(hStringID("font_sampler"), t2d);
        // rcd.setVertexBufferLayout(lo, 2);
        auto* rc = hRenderer::createRenderCall(rcd);
        pooledSprites.emplace_back(texture, rc);
    }

    static void renderSpriteGroup(hRenderer::hCmdList* cl, const RenderSprite* sprites, size_t count) {
        //for ()
    }
};

std::vector<RenderSprite::SpriteCall> RenderSprite::pooledSprites;

size_t g_RenderSpriteCount = 0;
RenderSprite g_RenderSprites[100];
RenderSprite* g_RenderSpriteFreelist = nullptr;

hRegisterComponentObjectType(RenderSprite, RenderSprite, Invader::proto::RenderSprite);
hBool RenderSprite::serialiseObject(Invader::proto::RenderSprite* obj) const {
    return hTrue;
}
hBool RenderSprite::deserialiseObject(Invader::proto::RenderSprite* obj) {
	loadingData = new RenderSprite::LoadingData();
    loadingData->spriteResourceID = hStringID(obj->atlasresource().c_str());
	loadingData->shaderResourceID = hStringID(obj->atlasresource().c_str());

    return hTrue;
}
hBool RenderSprite::linkObject() {
	spriteTex = 
    return hTrue;
}
static hEntityComponent* RenderSpriteCompConstruct(hEntityComponentHandle* handle_address) {
    auto* r = new RenderSprite();
    r->initilise(handle_address);
    return r;
}
static void RenderSpriteCompDestruct(hEntityComponent* ptr) {
    auto* rs_ptr = (RenderSprite*)ptr;
    delete rs_ptr;
}
static hInt RenderSpriteCompCompact() {
    return 0;
}

class InvaderGame : public IUnitTest {
    enum class GameState {
        FirstRun, 
        LoadingPkg,
        Game,
        EndGame,
        Unload,
    };

    hTimer timer_;
    GameState currentState;
    hEntityContext* invaderCtx;
    hUuid_t defenderID;

public:
    InvaderGame( Heart::hHeartEngine* engine ) 
        : IUnitTest( engine )
        , currentState(GameState::FirstRun)
        , invaderCtx(nullptr) {
        SetCanRender(hTrue);
        timer_.reset();
    }
    ~InvaderGame() {
    }

    const hChar* getHelpString() override {
        return "An example game of Space Invaders.";
    }

    virtual hUint32 RunUnitTest() override {
        if (timer_.elapsedSec() > 60.0f || getForceExitFlag()) {
            SetExitCode(UNIT_TEST_EXIT_CODE_OK);
        }

        switch(currentState) {
        case GameState::FirstRun: {
            hResourceManager::loadPackage("invader");
            currentState = GameState::LoadingPkg;
        } break;
        case GameState::LoadingPkg: {
            if (hResourceManager::getIsPackageLoaded("invader")) {
                hEntityFactory::hComponentMgt defenderSpawner = {
                    hObjectFactory::getObjectDefinition(hStringID("DefenderComp")),
                    DefenderCompConstruct, DefenderCompDestruct, DefenderCompCompact,
                };
                hEntityFactory::registerComponentManagement(defenderSpawner);
                hEntityFactory::hComponentMgt renderSpriteSpawner = {
                    hObjectFactory::getObjectDefinition(hStringID("RenderSprite")),
                    RenderSpriteCompConstruct, RenderSpriteCompDestruct, RenderSpriteCompCompact,
                };
                hEntityFactory::registerComponentManagement(renderSpriteSpawner);

                invaderCtx = hEntityFactory::createEntityContext("InvaderGame", nullptr, 0);
                defenderID = hEntityFactory::createEntity(invaderCtx, hUUID::generateUUID(), hResourceManager::weakResource<hEntityDef>(hStringID("/invader/defender")));
                currentState = GameState::Game;
            }
        } break;
        case GameState::Game: {} break;
        case GameState::EndGame: {} break;
        case GameState::Unload: {} break;
        }

        return 0;
    }

    Heart::hRenderer::hCmdList* RenderUnitTest() override {
        auto* cl = hRenderer::createCmdList();
        hRenderer::clear(cl, hColour(0.f, 0.0627f, 0.345f, 1.f), 1.f);
        if (!g_RenderSpriteFreelist) {
            RenderSprite::renderSpriteGroup(cl, g_RenderSprites, hArraySize(g_RenderSprites));
        } else {
            auto remain = g_RenderSpriteCount;
            auto* p = g_RenderSprites;
            auto* i = g_RenderSpriteFreelist;
            do {
                if (p - i > 0) {
                    remain -= (p - i);
                    RenderSprite::renderSpriteGroup(cl, p, (p - i));
                }
                p = i;
                i = i->lnext;
            } while (i != g_RenderSpriteFreelist && remain);
        }
        return cl;
    }
};

DEFINE_HEART_UNIT_TEST(InvaderGame);