/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"

namespace Heart {

struct hColour {
    hColour() {}
    hColour( hByte r, hByte g, hByte b, hByte a ){ r_ = r/255.f;    b_ = b/255.f;   g_ = g/255.f;   a_ = a/255.f; }
    hColour( hFloat r, hFloat g, hFloat b, hFloat a ){ r_ = r;  b_ = b; g_ = g; a_ = a; }
    hFloat r_,g_,b_,a_;

    operator hUint32 () const {
        return ((hByte)( r_ * 255.0f )) << 16 |  ((hByte)( g_ * 255.0f )) << 8 | ((hByte)( b_ * 255.0f )) | ((hByte)( a_ * 255.0f )) << 24;
    }

    hColour operator * ( hFloat s ) const {
        return hColour( r_*s, g_*s, b_*s, a_*s );
    }

    hColour operator + ( hColour c ) const {
        return hColour( r_+c.r_, g_+c.g_, b_+c.b_, a_+c.a_ );
    }

    hColour operator - ( hColour c ) const {
        return hColour( r_-c.r_, g_-c.g_, b_-c.b_, a_-c.a_ );
    }

    hColour operator * ( hColour c ) const {
        return hColour( r_*c.r_, g_*c.g_, b_*c.b_, a_*c.a_ );
    }
};

const hColour WHITE( 1.0f, 1.0f, 1.0f, 1.0f );
const hColour BLACK( 0.0f, 0.0f, 0.0f, 1.0f );

}