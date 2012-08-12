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

    struct HEARTCORE_SLIBEXPORT hRenderViewportTargetSetup
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
	
	class HEARTCORE_SLIBEXPORT hRendererCamera
	{
	public:

		hRendererCamera();
		virtual	~hRendererCamera();

        void					    Initialise( hRenderer* renderer );
		void					    SetFieldOfView( hFloat fovDegrees ) { fov = hmDegToRad( fovDegrees ); }
		void					    SetProjectionParams( hFloat Ratio, hFloat Near, hFloat Far );
		void					    SetOrthoParams( hFloat width, hFloat height, hFloat znear, hFloat zfar );
        void					    SetOrthoParams( hFloat left, hFloat top, hFloat right, hFloat bottom, hFloat znear, hFloat zfar );
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
        void                        UpdateParameters();

	private:

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