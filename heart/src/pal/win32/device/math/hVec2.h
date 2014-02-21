/********************************************************************

	filename: 	hVec2.h	
	
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

#ifndef hmVec2_h__
#define hmVec2_h__

namespace Heart
{
#if defined (HEART_USE_XNAMATH)
    typedef XMFLOAT2 hCPUVec2;
#else
    typedef DirectX::XMFLOAT2 hCPUVec2;
    using DirectX::XMVectorZero;
    using DirectX::XMVector2Length;
    using DirectX::XMVector2Normalize;
    using DirectX::XMVector2NormalizeEst;
    using DirectX::XMVectorAdd;
    using DirectX::XMVectorSubtract;
    using DirectX::XMVectorScale;
    using DirectX::XMVectorDivide;
    using DirectX::XMVector2Dot;
    using DirectX::XMVectorNegate;
    using DirectX::XMVector2Cross;
    using DirectX::XMVectorSet;
    using DirectX::XMVector2Equal;
    using DirectX::XMStoreFloat2;
    using DirectX::XMLoadFloat2;
    using DirectX::XMVector2LengthEst;
#endif

    struct hVec2
    {
        hVec128 v;
        
        hVec2() {}
        hVec2( const hVec128& rhs );
        explicit hVec2( const hCPUVec2& rhs );
        hVec2( hFloat x, hFloat y );
        hVec2& operator = ( const hCPUVec2& b );
        operator hCPUVec2 () const;
        operator hFloatInVec() const { return v; }
        operator hVec128() const { return v; }
        hVec128 Get128() const { return v; }
    };

}

#endif // hmVec2_h__