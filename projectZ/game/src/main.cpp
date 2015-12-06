/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "Heart.h"
#include "render/hImGuiRenderer.h"
#include "render/hRendererCamera.h"
#include "2d/hTileRenderer2D.h"
#include "projectZGame.h"
#include "zombieComponent.h"

namespace h = Heart;
namespace hr = Heart::hRenderer;

static hBool package_loaded = false;
static h::hRendererCamera main_camera;

void HEART_API HeartAppFirstLoaded(Heart::hHeartEngine*) {
}

void HEART_API HeartAppCoreAssetsLoaded(Heart::hHeartEngine* engine) {
    Heart::hResourceManager::loadPackage("persistent");
}

void HEART_API HeartAppMainUpate(Heart::hHeartEngine* engine) {
    Heart::ImGuiNewFrame(engine->GetSystem(), engine->getActionManager());

    h::hMatrix v = h::hMatrix::identity();
    hFloat wOver2 = (hFloat)engine->GetSystem()->getWindowWidth()*.5f;
    hFloat hOver2 =(hFloat)engine->GetSystem()->getWindowHeight()*.5f;
    main_camera.setOrthoParams(-wOver2, hOver2, wOver2, -hOver2, .0f, 1.f);
    main_camera.setViewport(h::hRelativeViewport(0.f, 0.f, 1.f, 1.f));
    main_camera.setViewMatrix(v);
    if (package_loaded) {
        h::hTileRenderer2D::setView(main_camera);
        h::hTileRenderer2D::transformPlanesToViewSpace();
    } else
        package_loaded = h::hResourceManager::getIsPackageLoaded("persistent");
//    ImGui::Begin();
//    ImGui::End();
}

void HEART_API HeartAppMainRender(Heart::hHeartEngine* engine) {
    auto* cl = h::ImGuiCurrentCommandList();
    hr::clear(cl, h::hColour(1.f, 0.f, 1.f, 1.f), 1.f);

    if (package_loaded) {
        h::hTileRenderer2D::updateDynamicRenderResources(cl);
        h::hTileRenderer2D::renderTilePlanes(cl);
    }

    ImGui::Render();
    hr::swapBuffers(cl);
    hr::submitFrame(cl);
}

hBool HEART_API HeartAppShutdownUpdate(Heart::hHeartEngine* engine) {
    hBool dontDeadStrip = hTrue;
    dontDeadStrip &= ProjectZGame::auto_object_registered;
    dontDeadStrip &= ZombieComponent::auto_object_registered;
    return hTrue && dontDeadStrip;
}

void HEART_API HeartAppOnShutdown(Heart::hHeartEngine* engine) {
}

int main(int argc, char **argv) {

    hHeartEngineCallbacks callbacks = {0};

    callbacks.firstLoaded_ = HeartAppFirstLoaded;
    callbacks.coreAssetsLoaded_ = HeartAppCoreAssetsLoaded;
    callbacks.mainUpdate_ = HeartAppMainUpate;
    callbacks.mainRender_ = HeartAppMainRender;
    callbacks.shutdownUpdate_ = HeartAppShutdownUpdate;
    callbacks.onShutdown_ = HeartAppOnShutdown;

    char* script = nullptr;
    hSize_t scriptlen = 0;
    FILE* file = fopen("projectz/scripts/config.lua", "rt");
    if (file) {
        fseek(file, 0, SEEK_END);
        scriptlen = ftell(file);
        fseek(file, 0, SEEK_SET);
        script = new char[scriptlen+1];
        scriptlen = fread(script, 1, scriptlen, file);
        script[scriptlen]=0;
        fclose(file);
        file = nullptr;
    }

    Heart::hHeartEngine* engine = hHeartInitEngine(&callbacks, script, scriptlen, argc, argv);

    ProjectZGame::registerComponent();
    ZombieComponent::registerComponent();

    while (hHeartDoMainUpdate(engine) != hErrorCode) {}

    hHeartShutdownEngine(engine);

    delete script;
    return 0;
}

#if defined (PLATFORM_WINDOWS)

#include <stdlib.h>

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    return main(__argc, __argv);
}

#endif
