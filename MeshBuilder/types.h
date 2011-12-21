/********************************************************************

	filename: 	types.h	
	
	Copyright (c) 16:1:2011 James Moran
	
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

#ifndef TYPES_H__
#define TYPES_H__

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned long u32;
typedef signed long s32;
typedef unsigned long long u64;
typedef signed long long s64;

struct Vec3
{
	float x,y,z;
};

struct Vec2
{
	float x,y;
};

struct Vec4
{
	float x,y,z,w;
};

struct AABB
{
	Vec3						c;
	float						r[ 3 ];// half widths across x, y & z
};

enum VertexFormat
{
	VF_POSITION			= 1,
	VF_NORMAL			= 1 << 1,
	VF_TANGENT			= 1 << 2,
	VF_BINORMAL			= 1 << 3,
	VF_UV1				= 1 << 4,
	VF_UV2				= 1 << 5,
	VF_UV3				= 1 << 6,
	VF_UV4				= 1 << 7,
	VF_COLOUR1			= 1 << 8,
	VF_COLOUR2			= 1 << 9,
	VF_COLOUR3			= 1 << 10,
	VF_COLOUR4			= 1 << 11,

	VF_MAX				
};

#define HMIN( a, b ) a < b ? a : b
#define HMAX( a, b ) a > b ? a : b
#define Hpi						(3.14159265358979323846f)// long winded PI
#define HDegsToRads( deg ) ( deg * ( Hpi / 180.0f )) 
#define HRadsToDegs( rad ) ( rad * ( 180.0f/ Hpi ))

inline void ComputeAABBFromPointSet( const Vec3* pPoints, u32 nPoints, AABB* out )
{
	float MinX, MinY, MinZ;
	float MaxX, MaxY, MaxZ;

	out->c.x = 0.0f;
	out->c.y = 0.0f;
	out->c.z = 0.0f;
	out->r[0] = 0.0f;
	out->r[1] = 0.0f;
	out->r[2] = 0.0f;

	if ( !nPoints || !pPoints )
	{
		return;
	}

	MinX = MaxX = pPoints[ 0 ].x;
	MinY = MaxY = pPoints[ 0 ].y;
	MinZ = MaxZ = pPoints[ 0 ].z;

	for ( u32 i = 0; i < nPoints; ++i )
	{
		MinX = HMIN( MinX, pPoints[ i ].x );
		MaxX = HMAX( MaxX, pPoints[ i ].x );
		MinY = HMIN( MinY, pPoints[ i ].y );
		MaxY = HMAX( MaxY, pPoints[ i ].y );
		MinZ = HMIN( MinZ, pPoints[ i ].z );
		MaxZ = HMAX( MaxZ, pPoints[ i ].z );
	}

	out->r[0] = (MaxX - MinX) / 2;
	out->r[1] = (MaxY - MinY) / 2;
	out->r[2] = (MaxZ - MinZ) / 2;

	out->c.x = MinX + ( out->r[0] );
	out->c.y = MinY + ( out->r[1] );
	out->c.z = MinZ + ( out->r[2] );

}

inline void ExpandAABBByAABB( AABB* a, const AABB& b )
{
	//NOTE: this can be quicker!
	Vec3 t[4];

	t[ 0 ].x = a->c.x + a->r[ 0 ];
	t[ 0 ].y = a->c.y + a->r[ 1 ];
	t[ 0 ].z = a->c.z + a->r[ 2 ];

	t[ 1 ].x = a->c.x - a->r[ 0 ];
	t[ 1 ].y = a->c.y - a->r[ 1 ];
	t[ 1 ].z = a->c.z - a->r[ 2 ];

	t[ 2 ].x = b.c.x + b.r[ 0 ];
	t[ 2 ].y = b.c.y + b.r[ 1 ];
	t[ 2 ].z = b.c.z + b.r[ 2 ];

	t[ 3 ].x = b.c.x - b.r[ 0 ];
	t[ 3 ].y = b.c.y - b.r[ 1 ];
	t[ 3 ].z = b.c.z - b.r[ 2 ];

	ComputeAABBFromPointSet( t, 4, a );
}

inline void ClearAABB( AABB* out )
{
	out->c.x = 0.0f;
	out->c.y = 0.0f;
	out->c.z = 0.0f;
	out->r[0] = 0.0f;
	out->r[1] = 0.0f;
	out->r[2] = 0.0f;
}

#ifdef _DEBUG
#define ENABLE_ASSERTS
#endif // _DEBUG

#ifdef ENABLE_ASSERTS

#include <assert.h>

#define ASSERT( x ) assert( x )
#define D3DV( x ) ASSERT( SUCCEEDED( x ) )

#else

#define D3DV( x )	x
#define ASSERT( x )

#endif // _DEBUG

#endif // TYPES_H__