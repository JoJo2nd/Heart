/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "minfs.h"
#include "builder.pb.h"
#include "resource_renderpipeline.pb.h"
#include "getopt.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
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
    { "render_pass", no_argument, 0, 'p' },
    { 0, 0, 0, 0 }
};

namespace hprp = Heart::proto::renderpipeline;

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
        case 'z': fprintf(stdout, "heart render pipeline builder v0.8.0"); exit(0);
        case 'v': verbose = true; break;
        case 'p': parse_render_pass = true; break;
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
    Heart::builder::Output resource_output;

    if (parse_render_pass) {
        rapidjson::Document pass_doc;
        /* -- TODO ?? --
        FILE* json_file = fopen(input_pb.resourceinputpath().c_str(), "rb");
        ensure_condition(json_file, "Unable to open file %s to parse render pass.", input_pb.resourceinputpath().c_str());
        auto json_size = minfs_get_file_size(input_pb.resourceinputpath().c_str());
        std::vector<char> json_data;
        json_data.resize(json_size, 0);
        fread(json_data.data(), 1, json_size, json_file);
        pass_doc.ParseInsitu(json_data.data());
        ensure_condition(!pass_doc.HasParseError(), "JSON parse error");
        ensure_condition(pass_doc.HasMember("inputs"), "missing inputs");
        ensure_condition(pass_doc.HasMember("outputs"), "missing outputs");
        ensure_condition(pass_doc.HasMember("techniquename"), "missing technique name");
        hprp::Pass pass_pb;
        pass_pb.set_techniquename(pass_doc["techniquename"].GetString());
        for (uint32_t output_idx=0,output_n=pass_doc["inputs"].Size(); output_idx<output_n; ++output_idx) {
            auto& input = pass_doc["inputs"][output_idx];
            ensure_condition(input.HasMember("name") && input.HasMember("type"), "input missing members 'name' and/or 'type'");
            auto* input_pb = pass_pb.add_inputs();
            input_pb->set_name(input["name"].GetString());
            hprp::InputType type_val;
            ensure_condition(hprp::InputType_Parse(input["type"].GetString(), &type_val), "Couldn't parse input type");
            input_pb->set_type(type_val);
        }
        for (uint32_t output_idx = 0, output_n = pass_doc["outputs"].Size(); output_idx<output_n; ++output_idx) {
            auto& output = pass_doc["outputs"][output_idx];
            ensure_condition(output.HasMember("name") && output.HasMember("type") && output.HasMember("index"), "output object missing members 'name' or 'type' or 'index'");
            auto* output_pb = pass_pb.add_outputs();
            output_pb->set_name(output["name"].GetString());
            output_pb->set_index(output["index"].GetUint());
            hprp::OutputType type_val;
            ensure_condition(hprp::OutputType_Parse(output["type"].GetString(), &type_val), "Couldn't parse output type");
            output_pb->set_type(type_val);
        }
        
        output.mutable_pkgdata()->set_type_name(pass_pb.GetTypeName());
        output.mutable_pkgdata()->set_messagedata(pass_pb.SerializeAsString());
        */
    } else { // parse the render pipeline
        rapidjson::Document pipeline_doc;
        FILE* json_file = fopen(input_pb.resourceinputpath().c_str(), "rb");
        ensure_condition(json_file, "Unable to open file %s to parse render pipeline.", input_pb.resourceinputpath().c_str());
        auto json_size = minfs_get_file_size(input_pb.resourceinputpath().c_str());
        std::vector<char> json_data;
        json_data.resize(json_size, 0);
        fread(json_data.data(), 1, json_size, json_file);
        pipeline_doc.ParseInsitu(json_data.data());
        ensure_condition(!pipeline_doc.HasParseError(), "JSON parse error");
        hprp::Pipeline pipeline_pb;
        const auto& pipeline = pipeline_doc["pipeline"];
        uint32_t totalbindings = 0;
        for (uint32_t stage_idx = 0, stage_n = pipeline["stages"].Size(); stage_idx<stage_n; ++stage_idx) {
            auto& stage = pipeline["stages"][stage_idx];
            auto* stage_pb = pipeline_pb.add_stages();
            ensure_condition(stage.HasMember("name") && stage.HasMember("technique") && stage.HasMember("view"), "pipeline stage missing members 'name', 'resource' or 'inputs'");
            stage_pb->set_name(stage["name"].GetString());
            stage_pb->set_viewname(stage["view"].GetString());
            stage_pb->set_technique(stage["technique"].GetString());
            if (stage.HasMember("outputs"))
            {
                for (uint32_t output_idx = 0, output_n = stage["outputs"].Size(); output_idx<output_n; ++output_idx) {
                    const auto& outputs = stage["outputs"][output_idx];
                    stage_pb->add_outputs(outputs.GetString());
                    resource_output.add_resourcedependency(outputs.GetString());
                }
            }

        }
        resource_output.mutable_pkgdata()->set_type_name(pipeline_pb.GetTypeName());
        resource_output.mutable_pkgdata()->set_messagedata(pipeline_pb.SerializeAsString());
    }

    google::protobuf::io::OstreamOutputStream filestream(&std::cout);
    return resource_output.SerializeToZeroCopyStream(&filestream) ? 0 : -2;
}

