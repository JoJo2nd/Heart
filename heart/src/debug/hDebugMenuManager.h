/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#define HDEBUGMENUMANAGER_H__ //todo !!JM
#ifndef HDEBUGMENUMANAGER_H__

#include "base/hTypes.h"
#include "base/hMap.h"
#include "render/hRendererCamera.h"

namespace Heart
{
    class hRenderSubmissionCtx;
    class hRenderBuffer;
    class hRenderMaterialManager;

    struct hDebugRenderParams
    {
        hUint32 rtWidth_;
        hUint32 rtHeight_;
    };

    class hDebugMenuBase : public hMapElement< hUint32, hDebugMenuBase >
    {
    public:
        hDebugMenuBase() 
            : visible_(hFalse)
        {
        }
        ~hDebugMenuBase() {}

        void SetVisible(hBool visible) { visible_ = visible; }
        hBool GetVisible() const { return visible_; }
        virtual void PreRenderUpdate() = 0;
        virtual void Render(hRenderSubmissionCtx* ctx, hRenderBuffer* instanceCB, const hDebugRenderParams& params) = 0;
        virtual void EndFrameUpdate() = 0;

    private:

        hBool visible_;
    };

    class hActionManager;

    class hDebugMenuManager
    {
    public:

        hDebugMenuManager();
        ~hDebugMenuManager();

        void                        Initialise(hRenderer* renderer, hActionManager* controllerManager);
        void                        Destroy();
        void                        RegisterMenu(const hChar* name, hDebugMenuBase* menu);
        void                        UnregisterMenu(hDebugMenuBase* menu);
        void                        SetMenuVisiablity(const hChar* name, hBool show);
        void                        PreRenderUpdate();
        void                        RenderMenus(hRenderSubmissionCtx* rndCtx, hRenderMaterialManager* matManager);
        void                        EndFrameUpdate();
        static hDebugMenuManager*   GetInstance() { return instance_; }
        hBool                       Ready() { return hTrue; }

    private:

        typedef hMap< hUint32, hDebugMenuBase > hMenuMap;

        static hDebugMenuManager*   instance_;
        hRenderer*                  renderer_;
        hRendererCamera             camera_;
        hActionManager*             actionManager_;
        hMenuMap                    menuMap_;
    };
}

#endif // HDEBUGMENUMANAGER_H__