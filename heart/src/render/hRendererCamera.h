/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "base/hTypes.h"
#include "base/hRendererConstants.h"
#include "math/hViewFrustum.h"
#include "math/hMatrix.h"
#include "math/hVec3.h"
#include "math/hMathUtil.h"

namespace Heart {

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class hRendererCamera {
    public:

        void                        setFieldOfView( hFloat fovDegrees ) { fov = hDegToRad( fovDegrees ); }
        void                        setProjectionParams( hFloat Ratio, hFloat Near, hFloat Far );
        void                        setOrthoParams( hFloat width, hFloat height, hFloat znear, hFloat zfar );
        void                        setOrthoParams( hFloat left, hFloat top, hFloat right, hFloat bottom, hFloat znear, hFloat zfar );
        void                        setViewMatrix( const hMatrix& m );
        const hMatrix&              getViewMatrix() { return viewMatrix; }
        hVec3                       getPosition() const { return viewMatrix.getTranslation(); }
        hVec3                       getViewDirection() const { return normalize(viewMatrix.getCol2().getXYZ()); }
        hVec3                       getUpDirection() const { return normalize(viewMatrix.getCol1().getXYZ()); }
        const hMatrix&              getProjectionMatrix() { return projectionMatrix; }
        hViewFrustum*               getViewFrustum() { return &frustum; }
        hVec3                       projectTo2D( const hVec3& point );
        void                        setViewport( const hRelativeViewport& vp ) { viewport = vp; }
        hRelativeViewport           getViewport() const { return viewport; }
        hFloat                      getFar() const { return farPlane; }
        hFloat                      getNear() const { return nearPlane; }
    private:

        hFloat                      fov;
        hFloat                      aspect;
        hFloat                      nearPlane;
        hFloat                      farPlane;
        hFloat                      orthoWidth;
        hFloat                      orthoHeight;
        hBool                       ortho;
        hMatrix                     viewMatrix;
        hMatrix                     projectionMatrix;
        hViewFrustum                frustum;
        hRelativeViewport           viewport;
    };
    
}
