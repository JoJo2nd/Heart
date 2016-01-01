/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "lib_uniform_buffer.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

namespace ub {
namespace {
const char* dataTypeNames[DataType::Max] = {
    "float1",
    "float2",
    "float3",
    "float4",

    "float22",
    "float23",
    "float24",
    "float32",
    "float33",
    "float34",
    "float42",
    "float43",
    "float44",

    "int1",
    "int2",
    "int3",
    "int4",

    "uint1",
    "uint2",
    "uint3",
    "uint4",
};
uint32_t dataTypeSizes[DataType::Max] = {
    16,
    16,
    16,
    16,

    32,
    32,
    32,
    48,
    48,
    48,
    64,
    64,
    64,

    16,
    16,
    16,
    16,

    16,
    16,
    16,
    16,
};
}

bool Descriptor::loadFromJSON(const char* json, size_t len) {
    error = true;
    jsonData.resize(len+1, 0);
    memcpy(jsonData.data(), json, len);
    rapidjson::Document doc;
    doc.ParseInsitu(jsonData.data());

    if (doc.HasParseError()) return false;

    for (auto i=doc.MemberBegin(), n=doc.MemberEnd(); i!=n; ++i) {
        if (!i->name.IsString() || !i->value.IsArray()) return false;
        name = i->name.GetString();
        elements.reserve(i->value.Size());
        for (uint32_t j=0, nj=i->value.Size(); j<nj; ++j) {
            const auto& el = i->value[j];
            Element new_el;
            if (!el.HasMember("name") || !el.HasMember("type")) return false;
            const char* type_val = el["type"].GetString();
            new_el.type = DataType::Max;
            for (size_t dt=0, dtn=(size_t)DataType::Max; dt<dtn; ++dt) {
                if (strcmp(dataTypeNames[dt], type_val) == 0) {
                    new_el.type = (DataType)dt;
                    break;
                }
            }
            if (new_el.type == DataType::Max) return false;
            new_el.name = el["name"].GetString();
            new_el.index = j;
            new_el.typeByteSize = dataTypeSizes[(uint32_t)new_el.type];
            elements.push_back(new_el);
        }

        break;
    }

    error = false;
    return true;
}

}