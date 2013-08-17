/********************************************************************

	filename: 	hVec4.h	
	
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

#ifndef hmVec4_h__
#define hmVec4_h__

namespace Heart
{
#ifdef HEART_USE_XNAMATH
    typedef XMFLOAT4 hCPUVec4;
#else
    typedef DirectX::XMFLOAT4 hCPUVec4;
    using DirectX::XMVectorZero;
    using DirectX::XMVector4Length;
    using DirectX::XMVector4LengthEst;
    using DirectX::XMVector4LengthSq;
    using DirectX::XMVector4Normalize;
    using DirectX::XMVector4NormalizeEst;
    using DirectX::XMVectorAdd;
    using DirectX::XMVectorSubtract;
    using DirectX::XMVectorScale;
    using DirectX::XMVectorDivide;
    using DirectX::XMVector4Dot;
    using DirectX::XMVectorNegate;
    using DirectX::XMVector4Cross;
    using DirectX::XMVectorSet;
    using DirectX::XMVector4Equal;
    using DirectX::XMStoreFloat4;
    using DirectX::XMLoadFloat4;
#endif

    struct hVec3;

    struct hVec4
    {
        hVec128 v;

        
        hVec4() {}
        hVec4( const hVec128& rhs );
        explicit hVec4( const hVec3& rhs );
        explicit hVec4( const hCPUVec4& rhs );
        hVec4(const hVec3& rhs, hFloat w);
        hVec4( hFloat x, hFloat y, hFloat z, hFloat w );
        hVec4& operator = ( const hCPUVec4& b );
        operator hCPUVec4 () const;
        operator hFloatInVec() const { return v; }
        operator hVec128() const { return v; }
        hVec128 Get128() const { return v; }
        hFloat getX() const { return hVec128GetX(v); }
        hFloat getY() const { return hVec128GetY(v); }
        hFloat getZ() const { return hVec128GetZ(v); }
        hFloat getW() const { return hVec128GetW(v); }
    };

}

#endif // hmVec4_h__