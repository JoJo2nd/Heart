/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "minfs.h"
#include "builder.pb.h"
#include "resource_uniform_buffer.pb.h"
#include "lib_uniform_buffer.h"
#include "getopt.h"
#include <fstream>
#include <memory>
#include <iostream>
#include <unordered_set>

#if defined (_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable:4244)
#   pragma warning(disable:4267)
#else
#   pragma error ("Unknown platform")
#endif
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/io/coded_stream.h"

#if defined (_MSC_VER)
#   pragma warning(pop)
#endif


#ifdef _WIN32
#   include <io.h>
#   include <fcntl.h>
#endif
#include <unordered_map>

static const char argopts[] = "vi:";
static struct option long_options[] = {
    { "version", no_argument, 0, 'z' },
    { 0, 0, 0, 0 }
};

namespace hpub = Heart::proto::uniformbuffer;

#define fatal_error_check(x, msg, ...) if (!(x)) {fprintf(stderr, msg, __VA_ARGS__); exit(-1);}
#define fatal_error(msg, ...) fatal_error_check(false, msg, __VA_ARGS__)
#define ensure_condition(x, msg, ...) fatal_error_check(x, msg, __VA_ARGS__)

int main(int argc, char* argv[]) {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
#endif
    using namespace Heart;
    google::protobuf::io::IstreamInputStream input_stream(&std::cin);
    Heart::builder::Input input_pb;

    int c;
    int option_index = 0;
    bool verbose = false, use_stdin = true, parse_render_pass = false;

    while ((c = gop_getopt_long(argc, argv, argopts, long_options, &option_index)) != -1) {
        switch (c) {
        case 'z': fprintf(stdout, "heart uniform buffer definition builder v0.8.0"); exit(0);
        case 'i': {
            std::ifstream input_file_stream;
            input_file_stream.open(optarg, std::ios_base::binary | std::ios_base::in);
            if (input_file_stream.is_open()) {
                google::protobuf::io::IstreamInputStream file_stream(&input_file_stream);
                input_pb.ParseFromZeroCopyStream(&file_stream);
                use_stdin = false;
            }
        } break;
        default: return 2;
        }
    }

    if (use_stdin) {
        input_pb.ParseFromZeroCopyStream(&input_stream);
    }

    //write the resource
    Heart::builder::Output output;
    FILE* json_file = fopen(input_pb.resourceinputpath().c_str(), "rb");
    ensure_condition(json_file, "Unable to open file %s to parse render pass.", input_pb.resourceinputpath().c_str());
    auto json_size = minfs_get_file_size(input_pb.resourceinputpath().c_str());
    std::vector<char> json_data;
    json_data.resize(json_size, 0);
    fread(json_data.data(), 1, json_size, json_file);
    fclose(json_file);
    json_file = nullptr;
    ub::Descriptor ubDesc;
    ensure_condition(ubDesc.loadFromJSON(json_data.data(), json_data.size()), "Failed to parse JSON file '%s'", input_pb.resourceinputpath().c_str());

    hpub::Descriptor ubDesc_pb;
    ubDesc_pb.set_descriptortypename(ubDesc.getName());

    auto type_convert = [](ub::DataType val) {
        switch (val) {
        case ub::DataType::Float1: return hpub::float1;
        case ub::DataType::Float2: return hpub::float2;
        case ub::DataType::Float3: return hpub::float3;
        case ub::DataType::Float4: return hpub::float4;
        case ub::DataType::Float22: return hpub::float22;
        case ub::DataType::Float23: return hpub::float23;
        case ub::DataType::Float24: return hpub::float24;
        case ub::DataType::Float32: return hpub::float32;
        case ub::DataType::Float33: return hpub::float33;
        case ub::DataType::Float34: return hpub::float34;
        case ub::DataType::Float42: return hpub::float42;
        case ub::DataType::Float43: return hpub::float43;
        case ub::DataType::Float44: return hpub::float44;
        case ub::DataType::Int1: return hpub::int1;
        case ub::DataType::Int2: return hpub::int2;
        case ub::DataType::Int3: return hpub::int3;
        case ub::DataType::Int4: return hpub::int4;
        case ub::DataType::Uint1: return hpub::uint1;
        case ub::DataType::Uint2: return hpub::uint2;
        case ub::DataType::Uint3: return hpub::uint3;
        case ub::DataType::Uint4: return hpub::uint4;
        }
        return hpub::float1;
    };

    uint32_t byte_offset = 0;
    for (size_t i=0, n=ubDesc.getElementCount(); i < n; ++i) {
        hpub::Element* ele_pb = ubDesc_pb.add_elements();
        const auto& ele = ubDesc.getElement(i);
        ele_pb->set_name(ele.name);
        ele_pb->set_index((uint32_t)i);
        ele_pb->set_offset(byte_offset);
        ele_pb->set_type(type_convert(ele.type));

        byte_offset += ele.typeByteSize;
    }
    ubDesc_pb.set_totalbytesize(byte_offset);

    output.mutable_pkgdata()->set_type_name(ubDesc_pb.GetTypeName());
    output.mutable_pkgdata()->set_messagedata(ubDesc_pb.SerializeAsString());

    google::protobuf::io::OstreamOutputStream filestream(&std::cout);
    return output.SerializeToZeroCopyStream(&filestream) ? 0 : -2;
}

