/********************************************************************
	created:	2008/12/24

	filename: 	Camera.h

	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef __HRCAMERA_H__
#define __HRCAMERA_H__

#include "hTypes.h"
#include "Common.h"
#include "hMath.h"
#include "hArray.h"
#include "hViewFrustum.h"
#include "hRenderSubmissionContext.h"
#include "hRenderMaterialManager.h"
#include "hRendererConstants.h"

namespace Heart
{
    class hRenderer;

    struct hRenderViewportTargetSetup
    {
        hUint32         width_;
        hUint32         height_;
        hUint32         nTargets_;
        hTextureFormat  targetFormat_;
        hTextureFormat  depthFormat_;
        hBool           hasDepthStencil_;
    };

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	class hRendererCamera
	{
	public:

		hRendererCamera();
		virtual	~hRendererCamera();

        void					    Initialise( hRenderer* renderer );
		void					    SetFieldOfView( hFloat fovDegrees ) { fov = hmDegToRad( fovDegrees ); }
		void					    SetProjectionParams( hFloat Ratio, hFloat Near, hFloat Far );
		void					    SetOrthoParams( hFloat width, hFloat height, hFloat znear, hFloat zfar );
		void					    SetViewMatrix( const hMatrix& m );
        const hMatrix&			    GetViewMatrix() { return viewMatrix_; }
		const hMatrix&			    GetProjectionMatrix() { return projectionMatrix_; }
        hViewFrustum*			    ViewFrustum() { return &frustum_; }
		hVec3					    ProjectTo2D( const hVec3& point );
		void					    SetViewport( const hViewport& vp ) { viewport_ = vp; }
		hViewport				    GetViewport() const { return viewport_; }
		hFloat					    GetFar() const { return far_; }
		hFloat					    GetNear() const { return near_; }
        hUint32                     GetTargetWidth() const { return setup_.width_; }
        hUint32                     GetTargetHeight() const { return setup_.height_; }
        void                        SetTechniquePass( const hRenderTechniqueInfo* tech ) { validTechnique_ = tech; }
        hUint32                     GetTechniqueMask() const { return validTechnique_ ? validTechnique_->mask_ : 0; }        
        void                        SetRenderTargetSetup( const hRenderViewportTargetSetup& desc );
        hTexture*                   GetRenderTarget( hUint32 idx ) const { return renderTargets_[idx]; }
        hTexture*                   GetDepthTarget() const { return depthTarget_; }
        hViewportShaderConstants*   GetViewportConstants() const { return viewportConstants_; }
        hdParameterConstantBlock*   GetViewportConstantBlock() const { return &sharedConstantParamters_[0]; };

	private:

        void                        UpdateParameters();

        static const hUint32        MAX_TARGETS = 4;

        const hRenderTechniqueInfo* validTechnique_;
		hFloat						fov;
		hFloat						aspect_;
		hFloat						near_;
		hFloat						far_;
		hBool						isOrtho_;
		hMatrix						viewMatrix_;
		hMatrix						projectionMatrix_;
		hViewFrustum				frustum_;
		hViewport					viewport_;
		hRenderer*					renderer_;
        hRenderViewportTargetSetup  setup_;
        hTexture*                   renderTargets_[MAX_TARGETS];
        hTexture*                   depthTarget_;
        hdParameterConstantBlock*   sharedConstantParamters_;
	    hViewportShaderConstants*   viewportConstants_;
        hInstanceConstants*         instanceConstants_;
	};
	
}

#endif //__HRCAMERA_H__