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
#include "render/hRenderer.h"
#include "render/hImGuiRenderer.h"
#include "UnitTestFactory.h"
#include "Invader.pb.h"

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

};
hRegisterComponentObjectType(RenderSprite, RenderSprite, Invader::proto::RenderSprite);
hBool RenderSprite::serialiseObject(Invader::proto::RenderSprite* obj) const {
    return hTrue;
}
hBool RenderSprite::deserialiseObject(Invader::proto::RenderSprite* obj) {
    return hTrue;
}
hBool RenderSprite::linkObject() {
    return hTrue;
}
static hEntityComponent* RenderSpriteCompConstruct(hEntityComponentHandle* handle_address) {
    return new RenderSprite();
}
static void RenderSpriteCompDestruct(hEntityComponent* ptr) {
    delete ptr;
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
        hRenderer::clear(cl, hColour(0.f, 0.2f, 0.0f, 1.f), 1.f);
        return cl;
    }
};

DEFINE_HEART_UNIT_TEST(InvaderGame);