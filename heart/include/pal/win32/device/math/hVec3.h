/********************************************************************

    filename: 	hVec3.h	
    
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

#ifndef hmVec3_h__
#define hmVec3_h__

namespace Heart
{
#if defined (HEART_USE_XNAMATH)
    typedef XMFLOAT3 hCPUVec3;
#else
    typedef DirectX::XMFLOAT3 hCPUVec3;
    using DirectX::XMVectorZero;
    using DirectX::XMVector3Length;
    using DirectX::XMVector3LengthEst;
    using DirectX::XMVector3LengthSq;
    using DirectX::XMVector3Normalize;
    using DirectX::XMVector3NormalizeEst;
    using DirectX::XMVectorAdd;
    using DirectX::XMVectorSubtract;
    using DirectX::XMVectorScale;
    using DirectX::XMVectorDivide;
    using DirectX::XMVector3Dot;
    using DirectX::XMVectorNegate;
    using DirectX::XMVector3Cross;
    using DirectX::XMVectorSet;
    using DirectX::XMVector3Equal;
    using DirectX::XMStoreFloat3;
    using DirectX::XMLoadFloat3;
    using DirectX::XMVector3Greater;
    using DirectX::XMVector3GreaterOrEqual;
    using DirectX::XMVector3Less;
    using DirectX::XMVector3LessOrEqual;
    using DirectX::XMVectorMultiply;
#endif

    struct hVec3
    {
        hVec128 v;

        
        hVec3() {}
        hVec3( const hVec128& rhs );
        explicit hVec3( const hCPUVec3& rhs );
        hVec3( hFloat x, hFloat y, hFloat z );
        hVec3& operator = ( const hCPUVec3& b );
        operator hCPUVec3 () const;
        operator hFloatInVec() const { return v; }
        operator hVec128() const { return v; }
        hVec128 Get128() const { return v; }
    };

}

#endif // hmVec3_h__