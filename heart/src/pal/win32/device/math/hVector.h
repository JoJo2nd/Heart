/********************************************************************

	filename: 	hVector.h	
	
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

#ifndef MATH_VECTOR_H__
#define MATH_VECTOR_H__

//using namespace Vectormath::Aos;
namespace Heart {
    typedef Vectormath::floatInVec  hFloat128;
    typedef Vectormath::boolInVec   hBool128;
    typedef Vectormath::Aos::Point3 hPoint3;
}

using Vectormath::Aos::absPerElem;
using Vectormath::Aos::affineInverse;
using Vectormath::Aos::appendScale;
using Vectormath::Aos::conj;
using Vectormath::Aos::copySignPerElem;
using Vectormath::Aos::cross;
using Vectormath::Aos::crossMatrix;
using Vectormath::Aos::crossMatrixMul;
using Vectormath::Aos::determinant;
using Vectormath::Aos::dist;
using Vectormath::Aos::distFromOrigin;
using Vectormath::Aos::distSqr;
using Vectormath::Aos::distSqrFromOrigin;
using Vectormath::Aos::divPerElem;
using Vectormath::Aos::dot;
using Vectormath::Aos::inverse;
using Vectormath::Aos::length;
using Vectormath::Aos::lengthSqr;
using Vectormath::Aos::lerp;
using Vectormath::Aos::loadXYZ;
using Vectormath::Aos::loadXYZArray;
using Vectormath::Aos::loadXYZW;
using Vectormath::Aos::maxElem;
using Vectormath::Aos::maxPerElem;
using Vectormath::Aos::minElem;
using Vectormath::Aos::minPerElem;
using Vectormath::Aos::mulPerElem;
using Vectormath::Aos::norm;
using Vectormath::Aos::normalize;
using Vectormath::Aos::normalizeApprox;
using Vectormath::Aos::orthoInverse;
using Vectormath::Aos::outer;
using Vectormath::Aos::prependScale;
using Vectormath::Aos::projection;
using Vectormath::Aos::recipPerElem;
using Vectormath::Aos::rotate;
using Vectormath::Aos::rowMul;
using Vectormath::Aos::rsqrtPerElem;
using Vectormath::Aos::scale;
using Vectormath::Aos::select;
using Vectormath::Aos::slerp;
using Vectormath::Aos::sqrtPerElem;
using Vectormath::Aos::squad;
using Vectormath::Aos::storeHalfFloats;
using Vectormath::Aos::storeXYZ;
using Vectormath::Aos::storeXYZArray;
using Vectormath::Aos::storeXYZW;
using Vectormath::Aos::sum;
using Vectormath::Aos::transpose;

#endif// MATH_VECTOR;