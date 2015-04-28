/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hTypes.h"
#include "base/hBase64.h"
#include "base/hClock.h"
#include "core/hHeart.h"
#include "render/hRenderer.h"
#include "render/hImGuiRenderer.h"
#include "UnitTestFactory.h"

using namespace Heart;

class ImGuiTestMenu : public IUnitTest {
    
	hTimer								timer_;


public:
    ImGuiTestMenu( Heart::hHeartEngine* engine ) 
        : IUnitTest( engine ) {
        SetCanRender(hTrue);
    }
    ~ImGuiTestMenu() {
    }

    virtual hUint32 RunUnitTest() override {
        if (timer_.elapsedMilliSec() > 120*1000 || getForceExitFlag()) {
            //SetExitCode(UNIT_TEST_EXIT_CODE_OK);
        }

        return 0;
    }

    void RenderUnitTest() override {
        hBool opened = hFalse;
        auto* cl = ImGuiNewFrame(engine_->GetSystem(), engine_->getActionManager());
        hRenderer::clear(cl, hColour(0.f, 0.f, 0.2f, 1.f), 1.f);
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);     // Normally user code doesn't need/want to call it because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
        ImGui::ShowTestWindow(&opened);
        ImGui::Render();
        hRenderer::swapBuffers(cl);
        hRenderer::submitFrame(cl);
    }
};

DEFINE_HEART_UNIT_TEST(ImGuiTestMenu);