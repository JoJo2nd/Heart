/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "Heart.h"
#include "render\hImGuiRenderer.h"
#include "projectZGame.h"
#include "zombieComponent.h"

void HEART_API HeartAppFirstLoaded(Heart::hHeartEngine*) {
}

void HEART_API HeartAppCoreAssetsLoaded(Heart::hHeartEngine* engine) {
    Heart::hResourceManager::loadPackage("persistent");
}

void HEART_API HeartAppMainUpate(Heart::hHeartEngine* engine) {
    Heart::ImGuiNewFrame(engine->GetSystem(), engine->getActionManager());

//    ImGui::Begin();
//    ImGui::End();
}

void HEART_API HeartAppMainRender(Heart::hHeartEngine* engine) {
    auto* cl = Heart::ImGuiCurrentCommandList();
    Heart::hRenderer::clear(cl, Heart::hColour(1.f, 0.f, 1.f, 1.f), 1.f);

    ImGui::Render();
    Heart::hRenderer::swapBuffers(cl);
    Heart::hRenderer::submitFrame(cl);
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