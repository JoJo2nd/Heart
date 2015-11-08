/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "render/hRendererCamera.h"
#include "render/hRenderer.h"
#include "render/hTextureResource.h"

namespace Heart
{
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRendererCamera::hRendererCamera() 
        : clear_(hFalse) {
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

    void hRendererCamera::Initialise()
    {
#if 0
        renderer_ = renderer;
        fov = HEART_PI / 4.0f;//45.0f
        viewMatrix_ = hMatrix::identity();
        isOrtho_ = hFalse;
        viewport_.x = 0.f;
        viewport_.y = 0.f;
        viewport_.w = 1.f;
        viewport_.h = 1.f;

        hZeroMem(&setup_, sizeof(setup_));

        cameraConstBlock_ = renderer_->GetMaterialManager()->GetGlobalConstantBlockByAlias("CameraConstants");
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    Heart::hViewport hRendererCamera::getTargetViewport() const {
#if 0
        if (setup_.targetTex_){
            hUint w=setup_.targetTex_->getWidth();
            hUint h=setup_.targetTex_->getHeight();
            return hViewport((hUint)(w*viewport_.x), (hUint)(h*viewport_.y), (hUint)(w*viewport_.w), (hUint)(h*viewport_.h));
        } else {
            //zero out
            return hViewport(0, 0, 1, 1);
        }
#else
        hStub();
        return hViewport(0, 0, 1, 1);
#endif
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

        projectionMatrix_ = hMatrix::perspective( fov, aspect_, near_, far_ );

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

        projectionMatrix_ = hMatrix::orthographic(0.f, 0.f, width, height, near_, far_);
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

        projectionMatrix_ = hMatrix::orthographic(left, right, bottom, top, near_, far_);
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
        viewProj = proj*view;
        ret = viewProj*point4; 
        //hVec3 r2( ((ret.x/ret.w) * viewport_.width_ / 2), ((ret.y/ret.w) * viewport_.height_ / 2), ret.z/ret.w );
        hVec3 r2 = ret.getXYZ() / ret.getW();
        
        return mulPerElem( r2, hVec3(vp.width_ / 2.f, vp.height_ / 2.f, 1.f));

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRendererCamera::bindRenderTargetSetup(const hRenderViewportTargetSetup& desc)
    {
#if 0
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
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRendererCamera::releaseRenderTargetSetup()
    {
#if 0
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
#else
        hStub();
#endif
    }

}
