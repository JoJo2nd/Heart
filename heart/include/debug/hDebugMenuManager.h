/********************************************************************

    filename: 	hDebugMenuManager.h	
    
    Copyright (c) 12:8:2012 James Moran
    
    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.
    
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
    
    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
    
    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
    
    3. This notice may not be removed or altered from any source
    distribution.

*********************************************************************/
#ifndef HDEBUGMENUMANAGER_H__
#define HDEBUGMENUMANAGER_H__

namespace Heart
{
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
        virtual void Render(hRenderSubmissionCtx* ctx, hdParameterConstantBlock* instanceCB, const hDebugRenderParams& params) = 0;
        virtual void EndFrameUpdate() = 0;

    private:

        hBool visible_;
    };

    class hDebugMenuManager
    {
    public:

        hDebugMenuManager();
        ~hDebugMenuManager();

        void                        Initialise(hRenderer* renderer, hResourceManager* resManager, hControllerManager* controllerManager);
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
        hResourceManager*           resourceManager_;
        hControllerManager*         controllerManager_;
        hMenuMap                    menuMap_;
    };
}

#endif // HDEBUGMENUMANAGER_H__