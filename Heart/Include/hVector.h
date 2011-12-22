/********************************************************************
	created:	2008/07/13
	created:	13:7:2008   9:32
	filename: 	hmVector.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef MATH_VECTOR_H__
#define MATH_VECTOR_H__

#include "hTypes.h"
#include <xnamath.h>

typedef XMVECTOR            hVec128;

namespace Heart
{

    hFORCEINLINE void hVectorLoad( hVec128& v, const hFloat* f )
    {
        v = XMLoadFloat( f );
    }

    hFORCEINLINE void hVectorStore( const hVec128& v, hFloat* f )
    {
        XMStoreFloat( f, v );
    }

    struct hFloatInVec
    {
        hVec128 v;

        hFloatInVec() {}
        hFloatInVec( const hVec128& rhs ) : v(rhs) {}

        hFORCEINLINE operator hFloat ()
        {
            hFloat a;
            hVectorStore( v, &a );
            return a;
        }

        hFORCEINLINE hFloatInVec& operator = ( hFloat b )
        {
            hVectorLoad( v, &b );
            return *this;
        }
    };
}

#endif// MATH_VECTOR;