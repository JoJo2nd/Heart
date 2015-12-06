/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "render/hRendererCamera.h"
#include "render/hRenderer.h"
#include "render/hTextureResource.h"

namespace Heart {

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRendererCamera::setViewMatrix( const hMatrix& m ) {
        viewMatrix = m;
        if (ortho) frustum.updateFromOrthoCamera(getPosition(), getViewDirection(), getUpDirection(), orthoWidth, orthoHeight, nearPlane, farPlane);
        else frustum.updateFromCamera(getPosition(), getViewDirection(), getUpDirection(), fov, aspect, nearPlane, farPlane);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRendererCamera::setProjectionParams( hFloat Ratio, hFloat Near, hFloat Far ) {
        aspect = Ratio;
        nearPlane = Near;
        farPlane = Far;
        ortho = hFalse;

        projectionMatrix = hMatrix::perspective( fov, aspect, nearPlane, farPlane );
        frustum.updateFromCamera(getPosition(), getViewDirection(), getUpDirection(), fov, aspect, nearPlane, farPlane);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRendererCamera::setOrthoParams( hFloat width, hFloat height, hFloat znear, hFloat zfar ) {
        aspect = width / height;
        nearPlane = znear;
        farPlane = zfar;
        ortho = hTrue;
        orthoWidth = width;
        orthoHeight = height;

        projectionMatrix = hMatrix::orthographic(0.f, width, 0.f, height, nearPlane, farPlane);
        frustum.updateFromOrthoCamera(getPosition(), getViewDirection(), getUpDirection(), orthoWidth, orthoHeight, nearPlane, farPlane);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRendererCamera::setOrthoParams( hFloat left, hFloat top, hFloat right, hFloat bottom, hFloat znear, hFloat zfar ) {
        aspect = (right-left) / (top-bottom);
        nearPlane = znear;
        farPlane = zfar;
        ortho = hTrue;
        orthoWidth = (right - left);
        orthoHeight = (top - bottom);

        projectionMatrix = hMatrix::orthographic(left, right, bottom, top, nearPlane, farPlane);
        frustum.updateFromOrthoCamera(getPosition(), getViewDirection(), getUpDirection(), orthoWidth, orthoHeight, nearPlane, farPlane);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hVec3 hRendererCamera::projectTo2D( const hVec3& point ) {
        hVec4 ret;
        hMatrix view, proj,viewProj;

        view = getViewMatrix();
        proj = getProjectionMatrix();
        hVec4 point4( point );
        viewProj = proj*view;
        ret = viewProj*point4; 
        hVec3 r2 = ret.getXYZ() / ret.getW();
        
        return r2;
    }
}
