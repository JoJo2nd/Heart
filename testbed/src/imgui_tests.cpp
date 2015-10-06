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
    
    hTimer timer_;


public:
    ImGuiTestMenu( Heart::hHeartEngine* engine ) 
        : IUnitTest( engine ) {
        SetCanRender(hTrue);
    }
    ~ImGuiTestMenu() {
    }

    const hChar* getHelpString() override {
        return "An example of the ImGui test menu in action.";
    }

    virtual hUint32 RunUnitTest() override {
        hBool opened = hTrue;
        ImGui::ShowTestWindow(&opened);
        if (!opened || getForceExitFlag()) {
            SetExitCode(UNIT_TEST_EXIT_CODE_OK);
        }

        return 0;
    }

    Heart::hRenderer::hCmdList* RenderUnitTest() override {
        auto* cl = hRenderer::createCmdList();
        hRenderer::clear(cl, hColour(0.f, 0.2f, 0.2f, 1.f), 1.f);
        return cl;
    }
};

DEFINE_HEART_UNIT_TEST(ImGuiTestMenu);
