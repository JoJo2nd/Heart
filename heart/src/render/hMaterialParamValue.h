/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "base/hStringID.h"

namespace Heart {

enum class hMaterialParamType {
    None,
    Resource,
    Float,
    Int,
};

class hMaterialParamValue {
    
    const void*         data_; // we never own this, so can never delete it.
    hUint               dataCount_;
    hMaterialParamType  type_;

public:
    hMaterialParamValue() 
        : type_(hMaterialParamType::None), data_(nullptr), dataCount_(0) {}
    hMaterialParamValue(hFloat* f, hUint count) 
        : type_(hMaterialParamType::Float), data_(f), dataCount_(count) {}
    hMaterialParamValue(hInt* i, hUint count) 
        : type_(hMaterialParamType::Int), data_(i), dataCount_(count) {}
    explicit hMaterialParamValue(hStringID resid) 
        : type_(hMaterialParamType::Resource), data_((void*)resid.id()), dataCount_(1) {}

    hMaterialParamType  type() const { return type_; }
    hUint               getCount() const { return dataCount_; }
    const hFloat*       getFloatData() const {
        if (type_!=hMaterialParamType::Float)
            return nullptr;
        return (hFloat*)data_;
    }
    const hInt*         getIntData() const {
        if (type_!=hMaterialParamType::Int)
            return nullptr;
        return (hInt*)data_;
    }
    hStringID           getResourceID() const {
        if (type_!=hMaterialParamType::Resource)
            return hStringID();
        return *((hStringID*)data_);
    }
};

}