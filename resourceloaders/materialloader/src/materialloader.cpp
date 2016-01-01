/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "minfs.h"
#include "resource_renderstate.pb.h"
#include "resource_material_fx.pb.h"
#include "builder.pb.h"
#include "getopt.h"

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

#include <fstream>
#include <vector>
#include <string>
#include <stdio.h>
#include <iostream>

#ifdef _WIN32
#   include <io.h>
#   include <fcntl.h>
#endif

static const char argopts[] = "vi:";
static struct option long_options[] = {
    { "version", no_argument, 0, 'z' },
    { 0, 0, 0, 0 }
};

#define fatal_error_check(x, msg, ...) if (!(x)) {fprintf(stderr, msg, __VA_ARGS__); exit(-1);}
#define fatal_error(msg, ...) fatal_error_check(false, msg, __VA_ARGS__)
#define ensure_condition(x, msg, ...) fatal_error_check(x, msg, __VA_ARGS__)

#define FS_MAX_PATH (260)

typedef std::vector< std::string > StrVectorType;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Heart::proto::MaterialSampler* findOrAddMaterialSampler(Heart::proto::MaterialResource* mat, const char* name) {
    for (uint32_t i=0, n=mat->samplers_size(); i<n; ++i) {
        if (strcmp(mat->samplers(i).samplername().c_str(), name) == 0) {
            return mat->mutable_samplers()->Mutable(i);
        }
    }
    auto r=mat->add_samplers();
    r->set_samplername(name);
    return r;
}

Heart::proto::MaterialParameter* findOrAddMaterialParameter(Heart::proto::MaterialResource* mat, const char* name) {
    for (uint32_t i=0, n=mat->parameters_size(); i<n; ++i) {
        if (strcmp(mat->parameters(i).paramname().c_str(), name) == 0) {
            return mat->mutable_parameters()->Mutable(i);
        }
    }
    auto r = mat->add_parameters();
    r->set_paramname(name);
    return r;
}

Heart::proto::MaterialTechnique* findMaterialTechnique(Heart::proto::MaterialResource* mat, const char* name) {
    for (uint32_t i=0, n=mat->techniques_size(); i<n; ++i) {
        if (strcmp(mat->techniques(i).techniquename().c_str(), name) == 0) {
            return mat->mutable_techniques()->Mutable(i);
        }
    }
    return nullptr;
}

Heart::proto::MaterialTechnique* findOrAddMaterialTechnique(Heart::proto::MaterialResource* mat, const char* name) {
    auto r = findMaterialTechnique(mat, name);
    if (r)
        return r;
    r = mat->add_techniques();
    r->set_techniquename(name);
    return r;
}

Heart::proto::MaterialPass* findOrAddMaterialPass(Heart::proto::MaterialTechnique* tech, uint32_t passidx) {
    if (tech->passes_size() > (int)passidx) {
        return tech->mutable_passes(passidx);
    }
    return tech->add_passes();
}

template< typename t_enum >
t_enum getEnumFromString(const char* v, bool(*fn)(const ::std::string&, t_enum*)) {
    t_enum r;
    ensure_condition(fn(v, &r), "Unable to parse enum value %s", v);
    return r;
}

void readMaterialJSONToMaterialData(const rapidjson::Document& root_json_doc, const char* json_path_base, Heart::proto::MaterialResource* mat, StrVectorType* includes, StrVectorType* deps) {
    using namespace Heart::proto;
    using namespace Heart::proto::renderstate;

    ensure_condition(root_json_doc.HasMember("material"), "Material root node is missing.");
    auto& json_doc = root_json_doc["material"];

    if (json_doc.HasMember("inherit")) {
        //Load the base first
        char full_path[FS_MAX_PATH];
        char new_path_base[FS_MAX_PATH];
        minfs_path_join(json_path_base, json_doc["inherit"].GetString(), full_path, FS_MAX_PATH);
        minfs_path_parent(full_path, new_path_base, FS_MAX_PATH);
        auto file_size = minfs_get_file_size(full_path);
        FILE* f = fopen(full_path, "rb");
        ensure_condition(f, "Unable to open file %s", full_path);
        std::vector<char> inc_json_data;
        inc_json_data.resize(file_size, 0);
        fread(inc_json_data.data(), 1, file_size, f);
        fclose(f);
        f = nullptr;
        rapidjson::Document inc_doc;
        inc_doc.Parse(inc_json_data.data());
        ensure_condition(!inc_doc.HasParseError(), "Failed to parse JSON file %s", full_path);
        includes->push_back(full_path);
        readMaterialJSONToMaterialData(inc_doc, new_path_base, mat, includes, deps);
    }
    if (json_doc.HasMember("samplers")) {
        for (uint32_t i = 0, n = json_doc["samplers"].Size(); i < n; ++i) {
            const auto& sampler = json_doc["samplers"][i];
            ensure_condition(sampler.HasMember("name"), "sampler is missing name");
            auto* sampler_pb = findOrAddMaterialSampler(mat, sampler["name"].GetString());;
            auto* sampler_def_pb = sampler_pb->mutable_samplerstate();
            sampler_pb->set_samplername(sampler["name"].GetString());
            if (sampler.HasMember("addressu")) sampler_def_pb->set_addressu(getEnumFromString(sampler["addressu"].GetString(), SamplerBorder_Parse));
            if (sampler.HasMember("addressv")) sampler_def_pb->set_addressv(getEnumFromString(sampler["addressv"].GetString(), SamplerBorder_Parse));
            if (sampler.HasMember("addressw")) sampler_def_pb->set_addressw(getEnumFromString(sampler["addressw"].GetString(), SamplerBorder_Parse));
            if (sampler.HasMember("filter")) sampler_def_pb->set_filter(getEnumFromString(sampler["filter"].GetString(), SamplerState_Parse));
            if (sampler.HasMember("maxanisotropy")) sampler_def_pb->set_maxanisotropy(sampler["maxanisotropy"].GetUint());
            if (sampler.HasMember("minlod")) sampler_def_pb->set_minlod((float)sampler["minlod"].GetDouble());
            if (sampler.HasMember("maxlod")) sampler_def_pb->set_maxlod((float)sampler["maxlod"].GetDouble());
            if (sampler.HasMember("miplodbias")) sampler_def_pb->set_miplodbias((float)sampler["miplodbias"].GetDouble());
            if (sampler.HasMember("bordercolour") && sampler["bordercolour"].IsArray() && sampler["bordercolour"].Size() == 4) {
                const auto& colour = sampler["bordercolour"];
                sampler_def_pb->mutable_bordercolour()->set_red((uint32_t)(colour[0].GetDouble()*255.f + .5f));
                sampler_def_pb->mutable_bordercolour()->set_green((uint32_t)(colour[1].GetDouble()*255.f + .5f));
                sampler_def_pb->mutable_bordercolour()->set_blue((uint32_t)(colour[2].GetDouble()*255.f + .5f));
                sampler_def_pb->mutable_bordercolour()->set_alpha((uint32_t)(colour[3].GetDouble()*255.f + .5f));
            }
        }
    }

    if (json_doc.HasMember("uniform_buffers")) {
        for (uint32_t i = 0, n = json_doc["uniform_buffers"].Size(); i < n; ++i) {
            const auto& unibuf = json_doc["uniform_buffers"][i];
            auto* unibuf_pb = mat->add_uniformbuffers();
            ensure_condition(unibuf.HasMember("resource") || unibuf.HasMember("embed"), "Material uniform buffer missing resource definition.");
            if (unibuf.HasMember("resource")) {
                unibuf_pb->set_resource(unibuf["resource"].GetString());
                deps->push_back(unibuf["resource"].GetString());
            }
            if (unibuf.HasMember("embed")) {
                unibuf_pb->set_embed(unibuf["embed"].GetBool());
            }
            if (unibuf.HasMember("mutable")) {
                unibuf_pb->set_allowoverride(unibuf["mutable"].GetBool());
            }
            if (unibuf.HasMember("shared")) {
                unibuf_pb->set_shared(unibuf["shared"].GetBool());
            }
        }
    }

    if (json_doc.HasMember("parameters")) {
        for (uint32_t i = 0, n = json_doc["parameters"].Size(); i < n; ++i) {
            const auto& parameter = json_doc["parameters"][i];
            ensure_condition(parameter.HasMember("name") && parameter.HasMember("type"), "parameter is missing name or type");
            auto* parameter_pb = findOrAddMaterialParameter(mat, parameter["name"].GetString());
            const auto& value = parameter["value"];
            switch (getEnumFromString(parameter["type"].GetString(), MaterialParameterType_Parse)) {
            case Heart::proto::matparam_float: {
                ensure_condition(parameter.HasMember("value") && parameter["value"].IsArray(), "parameter value is not an array");
                for (uint32_t pi = 0, pn = value.Size(); pi < pn; ++pi) {
                    parameter_pb->add_floatvalues((float)value[pi].GetDouble());
                }
            }break;
            case Heart::proto::matparam_int: {
                ensure_condition(parameter.HasMember("value") && parameter["value"].IsArray(), "parameter value is not an array");
                for (uint32_t pi = 0, pn = value.Size(); pi < pn; ++pi) {
                    parameter_pb->add_intvalues(value[pi].GetInt());
                }
            }break;
            case Heart::proto::matparam_texture: {
                parameter_pb->set_resourceid(value.GetString());
                deps->push_back(value.GetString());
                if (parameter.HasMember("mutable") && parameter["mutable"].GetBool()) {
                    parameter_pb->set_allowoverride(true);
                }
                deps->push_back(value.GetString());
            }break;
            }
        }
    }

    ensure_condition(json_doc.HasMember("techniques") && json_doc["techniques"].IsArray(), "material is missing techniques array");
    for (uint32_t ti = 0, tn = json_doc["techniques"].Size(); ti < tn; ++ti) {
        const auto& technique = json_doc["techniques"][ti];
        ensure_condition(technique.HasMember("name"), "technique is missing a name");
        ensure_condition(technique.HasMember("passes") && technique["passes"].IsArray(), "technique is missing passes array");
        auto* technique_pb = findOrAddMaterialTechnique(mat, technique["name"].GetString());
        for (uint32_t pi = 0, pn = technique["passes"].Size(); pi < pn; ++pi) {
            const auto& pass = technique["passes"][pi];
            auto* pass_pb = findOrAddMaterialPass(technique_pb, pi);
            if (pass.HasMember("blendenable")) pass_pb->mutable_blend()->set_blendenable(pass["blendenable"].GetBool());
            if (pass.HasMember("blendop")) pass_pb->mutable_blend()->set_blendop(getEnumFromString(pass["blendop"].GetString(), BlendFunction_Parse));
            if (pass.HasMember("blendopalpha")) pass_pb->mutable_blend()->set_blendopalpha(getEnumFromString(pass["blendopalpha"].GetString(), BlendFunction_Parse));
            if (pass.HasMember("destblend")) pass_pb->mutable_blend()->set_destblend(getEnumFromString(pass["destblend"].GetString(), BlendOp_Parse));
            if (pass.HasMember("srcblend")) pass_pb->mutable_blend()->set_srcblend(getEnumFromString(pass["srcblend"].GetString(), BlendOp_Parse));
            if (pass.HasMember("destblendalpha")) pass_pb->mutable_blend()->set_destblendalpha(getEnumFromString(pass["destblendalpha"].GetString(), BlendOp_Parse));
            if (pass.HasMember("srcblendalpha")) pass_pb->mutable_blend()->set_srcblendalpha(getEnumFromString(pass["srcblendalpha"].GetString(), BlendOp_Parse));
            if (pass.HasMember("rendertargetwritemask")) pass_pb->mutable_blend()->set_rendertargetwritemask(pass["rendertargetwritemask"].GetUint());

            if (pass.HasMember("depthtest")) pass_pb->mutable_depthstencil()->set_depthenable(pass["depthtest"].GetBool());
            if (pass.HasMember("depthfunc")) pass_pb->mutable_depthstencil()->set_depthfunc(getEnumFromString(pass["depthfunc"].GetString(), FunctionCompare_Parse));
            if (pass.HasMember("depthwrite")) pass_pb->mutable_depthstencil()->set_depthwritemask(pass["depthwrite"].GetBool());
            if (pass.HasMember("stencilenable")) pass_pb->mutable_depthstencil()->set_stencilenable(pass["stencilenable"].GetBool());
            if (pass.HasMember("stencilfunc")) pass_pb->mutable_depthstencil()->set_stencilfunc(getEnumFromString(pass["stencilfunc"].GetString(), FunctionCompare_Parse));
            if (pass.HasMember("stencildepthfailop")) pass_pb->mutable_depthstencil()->set_stencildepthfailop(getEnumFromString(pass["stencildepthfailop"].GetString(), StencilOp_Parse));
            if (pass.HasMember("stencilfail")) pass_pb->mutable_depthstencil()->set_stencilfailop(getEnumFromString(pass["stencilfail"].GetString(), StencilOp_Parse));
            if (pass.HasMember("stencilpass")) pass_pb->mutable_depthstencil()->set_stencilpassop(getEnumFromString(pass["stencilpass"].GetString(), StencilOp_Parse));
            if (pass.HasMember("stencilreadmask")) pass_pb->mutable_depthstencil()->set_stencilreadmask(pass["stencilreadmask"].GetUint());
            if (pass.HasMember("stencilwritemask")) pass_pb->mutable_depthstencil()->set_stencilwritemask(pass["stencilwritemask"].GetUint());
            if (pass.HasMember("stencilref")) pass_pb->mutable_depthstencil()->set_stencilref(pass["stencilenable"].GetUint());

            if (pass.HasMember("cullmode")) pass_pb->mutable_rasterizer()->set_cullmode(getEnumFromString(pass["cullmode"].GetString(), CullMode_Parse));
            if (pass.HasMember("depthbiasclamp")) pass_pb->mutable_rasterizer()->set_depthbiasclamp((float)pass["depthbiasclamp"].GetDouble());
            if (pass.HasMember("depthclipenable")) pass_pb->mutable_rasterizer()->set_depthclipenable(pass["depthclipenable"].GetBool());
            if (pass.HasMember("fillmode")) pass_pb->mutable_rasterizer()->set_fillmode(getEnumFromString(pass["fillmode"].GetString(), FillMode_Parse));
            if (pass.HasMember("slopescaleddepthbias")) pass_pb->mutable_rasterizer()->set_slopescaleddepthbias((float)pass["slopescaleddepthbias"].GetDouble());
            if (pass.HasMember("scissortest")) pass_pb->mutable_rasterizer()->set_scissorenable(pass["scissortest"].GetBool());

            if (pass.HasMember("vertex")) {
                pass_pb->set_vertex(pass["vertex"].GetString());
                deps->push_back(pass["vertex"].GetString());
            }
            if (pass.HasMember("pixel")) {
                pass_pb->set_pixel(pass["pixel"].GetString());
                deps->push_back(pass["pixel"].GetString());
            }
            if (pass.HasMember("fragment")) {
                pass_pb->set_pixel(pass["fragment"].GetString());
                deps->push_back(pass["fragment"].GetString());
            }
            if (pass.HasMember("geometry")) {
                pass_pb->set_geometry(pass["geometry"].GetString());
                deps->push_back(pass["geometry"].GetString());
            }
            if (pass.HasMember("hull")) {
                pass_pb->set_hull(pass["hull"].GetString());
                deps->push_back(pass["hull"].GetString());
            }
            if (pass.HasMember("domain")) {
                pass_pb->set_domain(pass["domain"].GetString());
                deps->push_back(pass["domain"].GetString());
            }
        }
    }
}

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
    bool verbose = false, use_stdin = true;

    while ((c = gop_getopt_long(argc, argv, argopts, long_options, &option_index)) != -1) {
        switch (c) {
        case 'z': fprintf(stdout, "heart material builder v0.8.0"); exit(0);
        case 'v': verbose = 1; break;
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

    Heart::proto::MaterialResource materialresource;
    std::string filepath;
    uint32_t filesize=0;
    StrVectorType depresnames;
    StrVectorType includes;

    filepath=input_pb.resourceinputpath();
    filesize=(uint32_t)minfs_get_file_size(filepath.c_str());
    char new_path_base[FS_MAX_PATH];
    minfs_path_parent(filepath.c_str(), new_path_base, FS_MAX_PATH);
    auto file_size = minfs_get_file_size(filepath.c_str());
    FILE* f = fopen(filepath.c_str(), "rb");
    ensure_condition(f, "Unable to open file %s", filepath.c_str());
    std::vector<char> inc_json_data;
    inc_json_data.resize(file_size, 0);
    fread(inc_json_data.data(), 1, file_size, f);
    fclose(f);
    f = nullptr;
    rapidjson::Document inc_doc;
    inc_doc.Parse(inc_json_data.data());
    if (inc_doc.HasParseError()) {
        std::string error_area(&inc_json_data[std::max(0ull, inc_doc.GetErrorOffset()-100)], std::min(inc_json_data.size()-inc_doc.GetErrorOffset(), 100ull));
        ensure_condition(!inc_doc.HasParseError(), "Failed to parse JSON file %s. Error '%s' at '%zu'. Looks like...\n%s", filepath.c_str(), rapidjson::GetParseError_En(inc_doc.GetParseError()), inc_doc.GetErrorOffset(), error_area.c_str());
    }
    readMaterialJSONToMaterialData(inc_doc, new_path_base, &materialresource, &includes, &depresnames);

    Heart::builder::Output output;
    output.add_filedependency(filepath);
    //write the resource header
    output.mutable_pkgdata()->set_type_name(materialresource.GetTypeName());
    output.mutable_pkgdata()->set_messagedata(materialresource.SerializeAsString());

    //Return a list of resources this material is dependent on
    for (const auto& i : depresnames) {
        output.add_resourcedependency(i);
    }
    for (const auto& i : includes) {
        output.add_filedependency(i);
    }

    google::protobuf::io::OstreamOutputStream filestream(&std::cout);
    return output.SerializeToZeroCopyStream(&filestream) ? 0 : -2;
}
