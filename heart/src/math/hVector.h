/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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