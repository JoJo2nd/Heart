/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef __HRCAMERA_H__
#define __HRCAMERA_H__

#include "base/hTypes.h"
#include "base/hRendererConstants.h"
#include "math/hViewFrustum.h"
#include "math/hMatrix.h"
#include "math/hVec3.h"
#include "math/hMathUtil.h"
#include "render/hRenderSubmissionContext.h"

namespace Heart
{
    
    class hTexture;
    class hRenderTargetView;
    class hDepthStencilView;
    class hRenderSubmissionCtx;
    struct hRenderTechniqueInfo;

    struct hRenderViewportTargetSetup
    {
        hUint               nTargets_;
        hTexture*           targetTex_; // used for dims- must not be NULL
        hRenderTargetView*  targets_[HEART_MAX_SIMULTANEOUS_RENDER_TARGETS];
        hDepthStencilView*  depth_;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class hRendererCamera
    {
    public:

        hRendererCamera();
        virtual ~hRendererCamera();

        void                        Initialise();
        void                        SetFieldOfView( hFloat fovDegrees ) { fov = hDegToRad( fovDegrees ); }
        void                        SetProjectionParams( hFloat Ratio, hFloat Near, hFloat Far );
        void                        SetOrthoParams( hFloat width, hFloat height, hFloat znear, hFloat zfar );
        void                        SetOrthoParams( hFloat left, hFloat top, hFloat right, hFloat bottom, hFloat znear, hFloat zfar );
        void                        SetViewMatrix( const hMatrix& m );
        const hMatrix&              GetViewMatrix() { return viewMatrix_; }
        const hMatrix&              GetProjectionMatrix() { return projectionMatrix_; }
        hViewFrustum*               ViewFrustum() { return &frustum_; }
        hVec3                       ProjectTo2D( const hVec3& point );
        void                        setViewport( const hRelativeViewport& vp ) { viewport_ = vp; }
        hRelativeViewport           getViewport() const { return viewport_; }
        hViewport                   getTargetViewport() const;
        hFloat                      GetFar() const { return far_; }
        hFloat                      GetNear() const { return near_; }
        void                        bindRenderTargetSetup( const hRenderViewportTargetSetup& desc );
        void                        releaseRenderTargetSetup();
        hUint                       getTargetCount() const { return setup_.nTargets_; }
        hRenderTargetView**         getTargets() { return setup_.targets_; }
        hRenderTargetView*          getRenderTarget( hUint32 idx ) const { 
            hcAssert(idx < setup_.nTargets_);
            return setup_.targets_[idx]; 
        }
        hDepthStencilView*          getDepthTarget() const { return setup_.depth_; }
        //hViewportShaderConstants*   GetViewportConstants() { return &viewportConstants_; }
        void                        UpdateParameters(hRenderSubmissionCtx* ctx);
        void                        setClearScreenFlag(hBool val) { clear_=val;}
        hBool                       getClearScreenFlag() const { return clear_;}
    private:

        hFloat                      fov;
        hFloat                      aspect_;
        hFloat                      near_;
        hFloat                      far_;
        hBool                       isOrtho_;
        hBool                       clear_;
        hMatrix                     viewMatrix_;
        hMatrix                     projectionMatrix_;
        hViewFrustum                frustum_;
        hRelativeViewport           viewport_;
        hRenderViewportTargetSetup  setup_;
        //hRenderBuffer*              cameraConstBlock_;
        //hViewportShaderConstants    viewportConstants_;
    };
    
}

#endif //__HRCAMERA_H__