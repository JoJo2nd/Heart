/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include <string>
#include <vector>

namespace ub {
enum class DataType {
    Float1,
    Float2,
    Float3,
    Float4,

    Float22,
    Float23,
    Float24,
    Float32,
    Float33,
    Float34,
    Float42,
    Float43,
    Float44,

    Int1,
    Int2,
    Int3,
    Int4,

    Uint1,
    Uint2,
    Uint3,
    Uint4,

    Max
};

struct Element {
    std::string name;
    DataType type;
    uint32_t typeByteSize;
    uint32_t index;
};

class Descriptor {
    std::string name;
    std::vector<Element> elements;
    std::vector<char> jsonData;
    bool error = true;
public:
    bool loadFromJSON(const char* json, size_t len);
    size_t getElementCount() const { return elements.size(); }
    const Element& getElement(size_t i) const { return elements[i]; }
    const std::string& getName() const { return name; }
};



}