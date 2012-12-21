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
        viewport_.x_ = 0;
        viewport_.y_ = 0;
        viewport_.width_ = 800;
        viewport_.height_ = 600;

        hZeroMem( renderTargets_, sizeof(renderTargets_) );
        depthTarget_ = NULL;

        cameraConstBlock_ = renderer_->GetMaterialManager()->GetGlobalConstantBlockByAlias("CameraConstants");
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRendererCamera::UpdateParameters(hRenderSubmissionCtx* ctx)
    {
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

        hConstBlockMapInfo map;
        ctx->Map(cameraConstBlock_, &map);
        hMemCpy(map.ptr, &viewportConstants_, sizeof(viewportConstants_));
        ctx->Unmap(&map);
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

        view = GetViewMatrix();
        proj = GetProjectionMatrix();
        hVec4 point4( point );
        viewProj = hMatrixFunc::mult( view, proj );
        ret = hMatrixFunc::mult( point4, viewProj ); 
        //hVec3 r2( ((ret.x/ret.w) * viewport_.width_ / 2), ((ret.y/ret.w) * viewport_.height_ / 2), ret.z/ret.w );
        hVec3 r2( hVec3Func::div( (hVec3)ret, hVec128SplatW( ret ) ) );
        
        return hVec3Func::componentMult( r2, hVec3( viewport_.width_ / 2.f, viewport_.height_ / 2.f, 1.f ) );

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRendererCamera::SetRenderTargetSetup( const hRenderViewportTargetSetup& desc )
    {
        hcAssertMsg( renderer_, "Call Initialise first!" );

        ReleaseRenderTargetSetup();

        setup_ = desc;
        hcAssert( setup_.nTargets_ < MAX_TARGETS );
        hZeroMem( renderTargets_, sizeof(renderTargets_) );
        depthTarget_ = NULL;

        for ( hUint32 i = 0; i < setup_.nTargets_; ++i )
        {
            renderer_->CreateTexture( setup_.width_, setup_.height_, 1, NULL, setup_.targetFormat_, RESOURCEFLAG_RENDERTARGET, GetGlobalHeap(), &renderTargets_[i] );
        }

        if ( setup_.hasDepthStencil_ )
        {
            renderer_->CreateTexture( setup_.width_, setup_.height_, 1, NULL, setup_.depthFormat_, RESOURCEFLAG_DEPTHTARGET, GetGlobalHeap(), &depthTarget_ );
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRendererCamera::ReleaseRenderTargetSetup()
    {
        for (hUint32 i = 0; i < MAX_TARGETS; ++i)
        {
            if (renderTargets_[i])
            {
                renderer_->DestroyTexture(renderTargets_[i]);
                renderTargets_[i] = NULL;
            }
        }
        if (depthTarget_)
        {
            renderer_->DestroyTexture(depthTarget_);
            depthTarget_ = NULL;
        }
    }

}