/********************************************************************

	filename: 	hPoly2.h	
	
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

#ifndef HPOLY2_H__
#define HPOLY2_H__


namespace Heart
{
	//////////////////////////////////////////////////////////////////////////
	// A generic polygon shape class class for 2D ////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	template< hUint32 _sides >
	class hPoly2
	{
	public:

		hVec2			verts_[ _sides ];
		hPlane			axes_[ _sides ];

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
			
		hPoly2()
		{

		}

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		~hPoly2()
		{
		}

	};

namespace Poly2Util
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	template < hUint32 _sides >
	static void		Build( const hVec2* points, hPoly2< _sides >& out )
	{
		hcAssertMsg( Validate( points, _sides ), "polygon has incorrect winding" );

		for ( hUint32 i = 0; i < _sides; ++i )
		{
			out.verts_[ i ] = points[ i ];
			hVec2 a = points[ (i+1)%_sides ] - points[ i ];
			hVec2 n( -a.y, a.x );

			Plane2::BuildPlane( n, points[ i ], out.axes_[ i ] );
		}
	}
	
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	static hBool	Validate( const hVec2* points, hUint32 nPoints )
	{
		for ( hUint32 i = 0; i < nPoints; ++i )
		{
			hVec2 a = points[ i ];
			hVec2 b = points[ (i+1) % nPoints ];
			hVec2 c = points[ (i+2) % nPoints ];

			if ( cross( b-a, c-b ).getX() > 0.0f )
			{
				return hFalse;
			}
		}

		return hTrue;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	template< hUint32 _sides1, hUint32 _sides2 >
	static hUint32	Intersect( const hPoly2< _sides1 >& a, const hPoly2< _sides2 >& b, hVec2* contacts ) 
	{
		hUint32 iVa[ _sides1 ] = { 0 };
		hUint32 iVb[ _sides2 ] = { 0 };
		hUint32 nContacts = 0;

		for ( hUint32 i = 0; i < _sides1; ++i )
		{
			const hPlane* axis = &a.axes_[ i ];
			hBool allInfront = hTrue;
			for ( hUint32 i2 = 0; i2 < _sides2; ++i2 )
			{
				hFloat t = distFromPlane( b.verts_[ i2 ], *axis );
				if ( t < 0.0f )
				{
					allInfront = hFalse;
				}
				else
				{
					iVb[ i2 ] = 1;
				}
			}

			if ( allInfront )
			{
				return 0;
			}
		}

		for ( hUint32 i = 0; i < _sides2; ++i )
		{
			const Plane2* axis = &b.axes_[ i ];
			hBool allInfront = hTrue;
			for ( hUint32 i2 = 0; i2 < _sides2; ++i2 )
			{
				hFloat t = distFromPlane( a.verts_[ i2 ], *axis );
				if ( t < 0.0f )
				{
					allInfront = hFalse;
				}
				else
				{
					iVa[ i2 ] = 1;
				}
			}

			if ( allInfront )
			{
				return 0;
			}
		}

		for ( hUint32 i = 0; i < _sides1; ++i )
		{
			hUint32 nI = ( i + 1 ) % _sides1;
			// if next vert is on the other side of the poly
			if ( iVa[ nI ] != iVa[ i ] )
			{
				for ( hUint32 i2 = 0; i2< _sides2; ++i2 )
				{
					hUint32 nI2 = ( i2 + 1 ) % _sides2;
					hFloat s, t;
					hVec2 cont1,cont2;
					if ( ClosestPointLineSegments( a.verts_[ i ], a.verts_[ nI ], b.verts_[ i2 ], b.verts_[ nI2 ], s, t, cont1, cont2 ) <= Limits::FLOAT_EPSILON )
					{
						// distance between the two contact points is 0.0f meaning the 
						// intersection is on the same point on both lines, i.e. contact point!
						contacts[ nContacts++ ] = cont1;
					}
				}
			}
		}

		for ( hUint32 i = 0; i < _sides2; ++i )
		{
			hUint32 nI = ( i + 1 ) % _sides2;
			// if next vert is on the other side of the poly
			if ( iVb[ nI ] != iVb[ i ] )
			{
				for ( hUint32 i2 = 0; i2< _sides1; ++i2 )
				{
					hUint32 nI2 = ( i2 + 1 ) % _sides2;
					hFloat s, t;
					hVec2 cont1,cont2;
					if ( ClosestPointLineSegments( b.verts_[ i ], b.verts_[ nI ], a.verts_[ i2 ], a.verts_[ nI2 ], s, t, cont1, cont2 ) <= Limits::FLOAT_EPSILON )
					{
						// distance between the two contact points is 0.0f meaning the 
						// intersection is on the same point on both lines, i.e. contact point!
						contacts[ nContacts++ ] = cont1;
					}
				}
			}
		}


		return nContacts;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	template< hUint32 _sides >
	static void		Transform( const hPoly2< _sides >& a, const hMatrix& m, hPoly2< _sides >& out )
	{
		for ( hUint32 i = 0; i < _sides; ++i )
		{
			out.verts_[ i ] = a.verts_[ i ] * m;
		}

		for ( hUint32 i = 0; i < _sides; ++i )
		{
			out.verts_[ i ] = out.verts_[ i ];
			hVec2 a = out.verts_[ (i+1)%_sides ] - out.verts_[ i ];
			hVec2 n( -a.y, a.x );

			buildPlane( n, out.verts_[ i ], out.axes_[ i ] );
		}
	}
}

}
#endif // HPOLY2_H__