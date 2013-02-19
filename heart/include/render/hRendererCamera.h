/********************************************************************

    filename: 	hRendererCamera.h	
    
    Copyright (c) 1:4:2012 James Moran
    
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

#ifndef __HRCAMERA_H__
#define __HRCAMERA_H__

namespace Heart
{
    class hRenderer;

    struct HEART_DLLEXPORT hRenderViewportTargetSetup
    {
        hUint           nTargets_;
        hTexture*       targets_[HEART_MAX_SIMULTANEOUS_RENDER_TARGETS];
        hTexture*       depth_;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    class HEART_DLLEXPORT hRendererCamera
    {
    public:

        hRendererCamera();
        virtual	~hRendererCamera();

        void                        Initialise( hRenderer* renderer );
        void                        SetFieldOfView( hFloat fovDegrees ) { fov = hmDegToRad( fovDegrees ); }
        void                        SetProjectionParams( hFloat Ratio, hFloat Near, hFloat Far );
        void                        SetOrthoParams( hFloat width, hFloat height, hFloat znear, hFloat zfar );
        void                        SetOrthoParams( hFloat left, hFloat top, hFloat right, hFloat bottom, hFloat znear, hFloat zfar );
        void                        SetViewMatrix( const hMatrix& m );
        const hMatrix&              GetViewMatrix() { return viewMatrix_; }
        const hMatrix&              GetProjectionMatrix() { return projectionMatrix_; }
        hViewFrustum*               ViewFrustum() { return &frustum_; }
        hVec3                       ProjectTo2D( const hVec3& point );
        void                        SetViewport( const hViewport& vp ) { viewport_ = vp; }
        hViewport                   GetViewport() const { return viewport_; }
        hFloat                      GetFar() const { return far_; }
        hFloat                      GetNear() const { return near_; }
        void                        SetTechniquePass( const hRenderTechniqueInfo* tech ) { validTechnique_ = tech; }
        hUint32                     GetTechniqueMask() const { return validTechnique_ ? validTechnique_->mask_ : 0; }
        void                        SetRenderTargetSetup( const hRenderViewportTargetSetup& desc );
        void                        ReleaseRenderTargetSetup();
        hUint                       getTargetCount() const { return setup_.nTargets_; }
        hTexture**                  getTargets() { return setup_.targets_; }
        hTexture*                   getRenderTarget( hUint32 idx ) const { 
            hcAssert(idx < setup_.nTargets_);
            return setup_.targets_[idx]; 
        }
        hTexture*                   getDepthTarget() const { return setup_.depth_; }
        hViewportShaderConstants*   GetViewportConstants() { return &viewportConstants_; }
        void                        UpdateParameters(hRenderSubmissionCtx* ctx);
    private:
        const hRenderTechniqueInfo* validTechnique_;
        hFloat                      fov;
        hFloat                      aspect_;
        hFloat                      near_;
        hFloat                      far_;
        hBool                       isOrtho_;
        hMatrix                     viewMatrix_;
        hMatrix                     projectionMatrix_;
        hViewFrustum                frustum_;
        hViewport                   viewport_;
        hRenderer*                  renderer_;
        hRenderViewportTargetSetup  setup_;
        hdParameterConstantBlock*   cameraConstBlock_;
        hViewportShaderConstants    viewportConstants_;
    };
    
}

#endif //__HRCAMERA_H__