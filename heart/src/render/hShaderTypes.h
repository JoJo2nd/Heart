/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "math/hVec4.h"
#include "math/hVector.h"
#include "math/hMatrix.h"
#include "base/hMemoryUtil.h"

struct float1 {
    union { 
        float x;
        Heart::hVec4 v;
    };
};

struct float2 {
    union {
        float x, y;
        Heart::hVec4 v;
    };
};

struct float3 {
    union {
        float x, y, w;
        Heart::hVec4 v;
    };
};

struct float4 {
    union {
        float x, y, z, w;
        Heart::hVec4 v;
    };

    float4& operator = (const Heart::hVec4& rhs) {
        v = rhs;
        return *this;
    }
};

struct float22 {
    float2 m[2];
};

struct float23 {
    float3 m[2];
};

struct float24 {
    float4 m[2];
};

struct float32 {
    float2 m[3];
};

struct float33 {
    float3 m[3];
};

struct float34 {
    float4 m[3];
};

struct float42 {
    float2 m[4];
};

struct float43 {
    float3 m[4];
};

struct float44 {
    float44() : vm() {}
    union {
        float4 m[4];
        Heart::hMatrix vm;
    };

    float44& operator = (const Heart::hMatrix& rhs) {
        vm = rhs;
        return *this;
    }
};

struct int1 {
    hInt32 x;
    hInt32 pad[3];
};

struct int2 {
    hInt32 x, y;
    hInt32 pad[2];
};

struct int3 {
    hInt32 x, y, w;
    hInt32 pad[1];
};

struct int4 {
    hInt32 x, y, z, w;
};

struct uint1 {
    hUint32 x;
    hUint32 pad[3];
};

struct uint2 {
    hUint32 x, y;
    hUint32 pad[2];
};

struct uint3 {
    hUint32 x, y, w;
    hUint32 pad[1];
};

struct uint4 {
    hUint32 x, y, z, w;
};
