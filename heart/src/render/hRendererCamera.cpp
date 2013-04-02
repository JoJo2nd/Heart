/********************************************************************

    filename: 	hRendererCamera.cpp	
    
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


namespace Heart
{
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRendererCamera::hRendererCamera() 
        : renderer_(NULL)
        , validTechnique_(NULL)
    {
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRendererCamera::~hRendererCamera()
    {
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRendererCamera::Initialise(hRenderer* renderer)
    {
        renderer_ = renderer;
        fov = hmPI / 4.0f;//45.0f
        viewMatrix_ = hMatrixFunc::identity();
        isOrtho_ = hFalse;
        viewport_.x = 0.f;
        viewport_.y = 0.f;
        viewport_.w = 1.f;
        viewport_.h = 1.f;

        hZeroMem(&setup_, sizeof(setup_));

        cameraConstBlock_ = renderer_->GetMaterialManager()->GetGlobalConstantBlockByAlias("CameraConstants");
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRendererCamera::UpdateParameters(hRenderSubmissionCtx* ctx)
    {
        hViewport vp=getTargetViewport();
        hVec3 eye = hMatrixFunc::getRow( viewMatrix_, 3 );
        hVec3 dir = hMatrixFunc::getRow( viewMatrix_, 2 );
        hVec3 up  = hMatrixFunc::getRow( viewMatrix_, 1 );
        frustum_.UpdateFromCamera( eye, dir, up, fov, aspect_, near_, far_, isOrtho_ );

        viewportConstants_.projection_ = projectionMatrix_;
        viewportConstants_.projectionInverse_ = hMatrixFunc::inverse(projectionMatrix_);
        viewportConstants_.view_ = viewMatrix_;
        viewportConstants_.viewInverse_ = hMatrixFunc::inverse( viewMatrix_ );
        viewportConstants_.viewInverseTranspose_ = hMatrixFunc::transpose( viewportConstants_.viewInverse_ );
        viewportConstants_.viewProjection_ = hMatrixFunc::mult(viewMatrix_, projectionMatrix_);
        viewportConstants_.viewProjectionInverse_ = hMatrixFunc::inverse(viewportConstants_.viewProjection_);
        viewportConstants_.viewportSize_[0]=(hFloat)vp.width_;
        viewportConstants_.viewportSize_[1]=(hFloat)vp.height_;
        viewportConstants_.viewportSize_[2]=0.f;
        viewportConstants_.viewportSize_[3]=0.f;

        hConstBlockMapInfo map;
        ctx->Map(cameraConstBlock_, &map);
        hMemCpy(map.ptr, &viewportConstants_, sizeof(viewportConstants_));
        ctx->Unmap(&map);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    Heart::hViewport hRendererCamera::getTargetViewport() const {
        if (setup_.targetTex_){
            hUint w=setup_.targetTex_->getWidth();
            hUint h=setup_.targetTex_->getHeight();
            return hViewport((hUint)(w*viewport_.x), (hUint)(h*viewport_.y), (hUint)(w*viewport_.w), (hUint)(h*viewport_.h));
        } else {
            //zero out
            return hViewport(0, 0, 1, 1);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRendererCamera::SetViewMatrix( const hMatrix& m )
    {
        viewMatrix_ = m;
        //UpdateParameters();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRendererCamera::SetProjectionParams( hFloat Ratio, hFloat Near, hFloat Far )
    {
        aspect_ = Ratio;
        near_ = Near;
        far_ = Far;
        isOrtho_ = hFalse;

        projectionMatrix_ = hMatrixFunc::perspectiveFOV( fov, aspect_, near_, far_ );

        //UpdateParameters();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRendererCamera::SetOrthoParams( hFloat width, hFloat height, hFloat znear, hFloat zfar )
    {
        aspect_ = width / height;
        near_ = znear;
        far_ = zfar;
        isOrtho_ = hTrue;

        projectionMatrix_ = hMatrixFunc::orthoProj( width, height, near_, far_ );

        //UpdateParameters();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRendererCamera::SetOrthoParams( hFloat left, hFloat top, hFloat right, hFloat bottom, hFloat znear, hFloat zfar )
    {
        aspect_ = (right-left) / (bottom-top);
        near_ = znear;
        far_ = zfar;
        isOrtho_ = hTrue;

        projectionMatrix_ = hMatrixFunc::orthoProjOffCentre( left, right, bottom, top, near_, far_ );

        //UpdateParameters();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hVec3 hRendererCamera::ProjectTo2D( const hVec3& point )
    {
        hVec4 ret;
        hMatrix view, proj,viewProj;
        hViewport vp=getTargetViewport();

        view = GetViewMatrix();
        proj = GetProjectionMatrix();
        hVec4 point4( point );
        viewProj = hMatrixFunc::mult( view, proj );
        ret = hMatrixFunc::mult( point4, viewProj ); 
        //hVec3 r2( ((ret.x/ret.w) * viewport_.width_ / 2), ((ret.y/ret.w) * viewport_.height_ / 2), ret.z/ret.w );
        hVec3 r2( hVec3Func::div( (hVec3)ret, hVec128SplatW( ret ) ) );
        
        return hVec3Func::componentMult( r2, hVec3( vp.width_ / 2.f, vp.height_ / 2.f, 1.f ) );

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRendererCamera::bindRenderTargetSetup(const hRenderViewportTargetSetup& desc)
    {
        hcAssertMsg( renderer_, "Call Initialise first!" );
        hcAssertMsg(desc.targetTex_, "Must supply a target texture, can be depth or colour target");
        releaseRenderTargetSetup();
        setup_ = desc;
        setup_.targetTex_->AddRef();
        for (hUint32 i = 0; i < setup_.nTargets_; ++i) {
            if (setup_.targets_[i]) {
                setup_.targets_[i]->AddRef();
            }
        }
        if (setup_.depth_) {
            setup_.depth_->AddRef();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRendererCamera::releaseRenderTargetSetup()
    {
        if (setup_.targetTex_) {
            setup_.targetTex_->DecRef();
        }
        for (hUint32 i = 0; i < setup_.nTargets_; ++i) {
            if (setup_.targets_[i]) {
                setup_.targets_[i]->DecRef();
            }
        }
        if (setup_.depth_) {
            setup_.depth_->DecRef();
        }
        hZeroMem(&setup_, sizeof(setup_));
    }

}