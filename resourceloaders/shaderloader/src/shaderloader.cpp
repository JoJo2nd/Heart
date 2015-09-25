/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#if defined (PLATFORM_WINDOWS)
#   include <d3d11.h>
#   include <d3d11shader.h>
#   include <d3dcompiler.h>
#endif
#include "GL/glew.h"
#include "SDL.h"
#include "resource_common.pb.h"
#include "resource_shader.pb.h"
#include "builder.pb.h"
#include "minfs.h"
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
#include <regex>
#include <iostream>
#ifdef _WIN32
#   include <io.h>
#   include <fcntl.h>
#   define snprintf _snprintf
#endif
#ifndef MAX_PATH
#   define MAX_PATH 260
#endif

static const char argopts[] = "vi:";
static struct option long_options[] = {
    { "version", no_argument, 0, 'z' },
    { 0, 0, 0, 0 }
};

typedef unsigned char   uchar;
typedef unsigned int    uint;
typedef std::regex_iterator<std::string::iterator> sre_iterator;

#define fatal_error_check(x, msg, ...) if (!(x)) {fprintf(stderr, msg, __VA_ARGS__); exit(-1);}
#define fatal_error(msg, ...) fatal_error_check(false, msg, __VA_ARGS__)

#define VS_COMMON_DEFINES ""
#define PS_COMMON_DEFINES ""


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct ShaderDefine
{
    ShaderDefine()
    {}
    ShaderDefine(const char* define, const char* value)
        : define_(define), value_(value)
    {}
    std::string define_;
    std::string value_;
};

struct ShaderCompileParams
{
    std::string                 entry_;
    Heart::proto::eShaderType   profile_;
    uint                        compileFlags_;
    std::vector<ShaderDefine>   macros_;
};

typedef uint (*CompilerFunc)(std::string*, const ShaderCompileParams&, std::string*, void**, size_t*);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#if defined (PLATFORM_WINDOWS)
size_t parseVertexInputFormat(const D3D11_SHADER_DESC &desc, ID3D11ShaderReflection* reflect, Heart::proto::ShaderResource* shaderresource);
uint compileD3DShader(std::string* shader_source, const ShaderCompileParams& shader_params,
    std::string* out_errors, void** bin_blob, size_t* bin_blob_len);
#endif
uint initGLCompiler(std::string* out_errors);
uint compileGLShader(std::string* shader_source, const ShaderCompileParams& shader_params,
    std::string* out_errors, void** bin_blob, size_t* bin_blob_len);
uint parseShaderSource(const std::string& shader_path, std::vector<std::string> in_include_paths,
    std::string* out_source_string, std::map<std::string, std::string>* inc_ctx, std::vector<std::string>*);

struct ShaderProfile {
    struct Define {
        const char* def;
        const char* value;
    };
    const char* profileName;
    std::vector<Define> defineString;
    Heart::proto::eShaderType type;
    CompilerFunc func_;
    Heart::proto::eShaderRenderSystem system_;
} shaderProfiles[] = {
    { "XXXXXXX",      { { "HEART_INVALID"                 , "0"}, { "HEART_XXX" , "0"} },   Heart::proto::eShaderType_ES2_vs    , nullptr ,         Heart::proto::eShaderRenderSystem_OpenGL, },

    { "ES2_vs",       { {"HEART_COMPILE_VERTEX_PROG", "1"}, {"HEART_COMPILE_FRAGMENT_PROG", "0"}, {"HEART_COMPILE_GEOMETRY_PROG", "0"}, {"HEART_COMPILE_COMPUTE_PROG", "0"}, {"HEART_COMPILE_HULL_PROG", "0"}, {"HEART_COMPILE_DOMAIN_PROG", "0"}, {"HEART_ES2", "1"}, {"HEART_WebGL", "0"}, {"HEART_ES3", "0"}, {"HEART_FL10", "0"}, {"HEART_FL11", "0"} },  Heart::proto::eShaderType_ES2_vs    , compileGLShader , Heart::proto::eShaderRenderSystem_OpenGL, },
    { "ES2_ps",       { {"HEART_COMPILE_VERTEX_PROG", "0"}, {"HEART_COMPILE_FRAGMENT_PROG", "1"}, {"HEART_COMPILE_GEOMETRY_PROG", "0"}, {"HEART_COMPILE_COMPUTE_PROG", "0"}, {"HEART_COMPILE_HULL_PROG", "0"}, {"HEART_COMPILE_DOMAIN_PROG", "0"}, {"HEART_ES2", "1"}, {"HEART_WebGL", "0"}, {"HEART_ES3", "0"}, {"HEART_FL10", "0"}, {"HEART_FL11", "0"} },  Heart::proto::eShaderType_ES2_ps    , compileGLShader , Heart::proto::eShaderRenderSystem_OpenGL, },
    { "WebGL_vs",     { {"HEART_COMPILE_VERTEX_PROG", "1"}, {"HEART_COMPILE_FRAGMENT_PROG", "0"}, {"HEART_COMPILE_GEOMETRY_PROG", "0"}, {"HEART_COMPILE_COMPUTE_PROG", "0"}, {"HEART_COMPILE_HULL_PROG", "0"}, {"HEART_COMPILE_DOMAIN_PROG", "0"}, {"HEART_ES2", "0"}, {"HEART_WebGL", "1"}, {"HEART_ES3", "0"}, {"HEART_FL10", "0"}, {"HEART_FL11", "0"} },  Heart::proto::eShaderType_WebGL_vs  , compileGLShader , Heart::proto::eShaderRenderSystem_OpenGL, },
    { "WebGL_ps",     { {"HEART_COMPILE_VERTEX_PROG", "0"}, {"HEART_COMPILE_FRAGMENT_PROG", "1"}, {"HEART_COMPILE_GEOMETRY_PROG", "0"}, {"HEART_COMPILE_COMPUTE_PROG", "0"}, {"HEART_COMPILE_HULL_PROG", "0"}, {"HEART_COMPILE_DOMAIN_PROG", "0"}, {"HEART_ES2", "0"}, {"HEART_WebGL", "1"}, {"HEART_ES3", "0"}, {"HEART_FL10", "0"}, {"HEART_FL11", "0"} },  Heart::proto::eShaderType_WebGL_ps  , compileGLShader , Heart::proto::eShaderRenderSystem_OpenGL, },
    { "ES3_vs",       { {"HEART_COMPILE_VERTEX_PROG", "1"}, {"HEART_COMPILE_FRAGMENT_PROG", "0"}, {"HEART_COMPILE_GEOMETRY_PROG", "0"}, {"HEART_COMPILE_COMPUTE_PROG", "0"}, {"HEART_COMPILE_HULL_PROG", "0"}, {"HEART_COMPILE_DOMAIN_PROG", "0"}, {"HEART_ES2", "0"}, {"HEART_WebGL", "0"}, {"HEART_ES3", "1"}, {"HEART_FL10", "0"}, {"HEART_FL11", "0"} },  Heart::proto::eShaderType_ES3_vs    , compileGLShader , Heart::proto::eShaderRenderSystem_OpenGL, },
    { "ES3_ps",       { {"HEART_COMPILE_VERTEX_PROG", "0"}, {"HEART_COMPILE_FRAGMENT_PROG", "1"}, {"HEART_COMPILE_GEOMETRY_PROG", "0"}, {"HEART_COMPILE_COMPUTE_PROG", "0"}, {"HEART_COMPILE_HULL_PROG", "0"}, {"HEART_COMPILE_DOMAIN_PROG", "0"}, {"HEART_ES2", "0"}, {"HEART_WebGL", "0"}, {"HEART_ES3", "1"}, {"HEART_FL10", "0"}, {"HEART_FL11", "0"} },  Heart::proto::eShaderType_ES3_ps    , compileGLShader , Heart::proto::eShaderRenderSystem_OpenGL, },
    { "FL10_vs",      { {"HEART_COMPILE_VERTEX_PROG", "1"}, {"HEART_COMPILE_FRAGMENT_PROG", "0"}, {"HEART_COMPILE_GEOMETRY_PROG", "0"}, {"HEART_COMPILE_COMPUTE_PROG", "0"}, {"HEART_COMPILE_HULL_PROG", "0"}, {"HEART_COMPILE_DOMAIN_PROG", "0"}, {"HEART_ES2", "0"}, {"HEART_WebGL", "0"}, {"HEART_ES3", "0"}, {"HEART_FL10", "1"}, {"HEART_FL11", "0"} },  Heart::proto::eShaderType_FL10_vs   , compileGLShader , Heart::proto::eShaderRenderSystem_OpenGL, },
    { "FL10_ps",      { {"HEART_COMPILE_VERTEX_PROG", "0"}, {"HEART_COMPILE_FRAGMENT_PROG", "1"}, {"HEART_COMPILE_GEOMETRY_PROG", "0"}, {"HEART_COMPILE_COMPUTE_PROG", "0"}, {"HEART_COMPILE_HULL_PROG", "0"}, {"HEART_COMPILE_DOMAIN_PROG", "0"}, {"HEART_ES2", "0"}, {"HEART_WebGL", "0"}, {"HEART_ES3", "0"}, {"HEART_FL10", "1"}, {"HEART_FL11", "0"} },  Heart::proto::eShaderType_FL10_ps   , compileGLShader , Heart::proto::eShaderRenderSystem_OpenGL, },
    { "FL10_gs",      { {"HEART_COMPILE_VERTEX_PROG", "0"}, {"HEART_COMPILE_FRAGMENT_PROG", "0"}, {"HEART_COMPILE_GEOMETRY_PROG", "1"}, {"HEART_COMPILE_COMPUTE_PROG", "0"}, {"HEART_COMPILE_HULL_PROG", "0"}, {"HEART_COMPILE_DOMAIN_PROG", "0"}, {"HEART_ES2", "0"}, {"HEART_WebGL", "0"}, {"HEART_ES3", "0"}, {"HEART_FL10", "1"}, {"HEART_FL11", "0"} },  Heart::proto::eShaderType_FL10_gs   , compileGLShader , Heart::proto::eShaderRenderSystem_OpenGL, },
    { "FL10_cs",      { {"HEART_COMPILE_VERTEX_PROG", "0"}, {"HEART_COMPILE_FRAGMENT_PROG", "0"}, {"HEART_COMPILE_GEOMETRY_PROG", "0"}, {"HEART_COMPILE_COMPUTE_PROG", "1"}, {"HEART_COMPILE_HULL_PROG", "0"}, {"HEART_COMPILE_DOMAIN_PROG", "0"}, {"HEART_ES2", "0"}, {"HEART_WebGL", "0"}, {"HEART_ES3", "0"}, {"HEART_FL10", "1"}, {"HEART_FL11", "0"} },  Heart::proto::eShaderType_FL10_cs   , compileGLShader , Heart::proto::eShaderRenderSystem_OpenGL, },
    { "FL11_vs",      { {"HEART_COMPILE_VERTEX_PROG", "1"}, {"HEART_COMPILE_FRAGMENT_PROG", "0"}, {"HEART_COMPILE_GEOMETRY_PROG", "0"}, {"HEART_COMPILE_COMPUTE_PROG", "0"}, {"HEART_COMPILE_HULL_PROG", "0"}, {"HEART_COMPILE_DOMAIN_PROG", "0"}, {"HEART_ES2", "0"}, {"HEART_WebGL", "0"}, {"HEART_ES3", "0"}, {"HEART_FL10", "0"}, {"HEART_FL11", "1"} },  Heart::proto::eShaderType_FL11_vs   , compileGLShader , Heart::proto::eShaderRenderSystem_OpenGL, },
    { "FL11_ps",      { {"HEART_COMPILE_VERTEX_PROG", "0"}, {"HEART_COMPILE_FRAGMENT_PROG", "1"}, {"HEART_COMPILE_GEOMETRY_PROG", "0"}, {"HEART_COMPILE_COMPUTE_PROG", "0"}, {"HEART_COMPILE_HULL_PROG", "0"}, {"HEART_COMPILE_DOMAIN_PROG", "0"}, {"HEART_ES2", "0"}, {"HEART_WebGL", "0"}, {"HEART_ES3", "0"}, {"HEART_FL10", "0"}, {"HEART_FL11", "1"} },  Heart::proto::eShaderType_FL11_ps   , compileGLShader , Heart::proto::eShaderRenderSystem_OpenGL, },
    { "FL11_gs",      { {"HEART_COMPILE_VERTEX_PROG", "0"}, {"HEART_COMPILE_FRAGMENT_PROG", "0"}, {"HEART_COMPILE_GEOMETRY_PROG", "1"}, {"HEART_COMPILE_COMPUTE_PROG", "0"}, {"HEART_COMPILE_HULL_PROG", "0"}, {"HEART_COMPILE_DOMAIN_PROG", "0"}, {"HEART_ES2", "0"}, {"HEART_WebGL", "0"}, {"HEART_ES3", "0"}, {"HEART_FL10", "0"}, {"HEART_FL11", "1"} },  Heart::proto::eShaderType_FL11_gs   , compileGLShader , Heart::proto::eShaderRenderSystem_OpenGL, },
    { "FL11_cs",      { {"HEART_COMPILE_VERTEX_PROG", "0"}, {"HEART_COMPILE_FRAGMENT_PROG", "0"}, {"HEART_COMPILE_GEOMETRY_PROG", "0"}, {"HEART_COMPILE_COMPUTE_PROG", "1"}, {"HEART_COMPILE_HULL_PROG", "0"}, {"HEART_COMPILE_DOMAIN_PROG", "0"}, {"HEART_ES2", "0"}, {"HEART_WebGL", "0"}, {"HEART_ES3", "0"}, {"HEART_FL10", "0"}, {"HEART_FL11", "1"} },  Heart::proto::eShaderType_FL11_cs   , compileGLShader , Heart::proto::eShaderRenderSystem_OpenGL, },
    { "FL11_hs",      { {"HEART_COMPILE_VERTEX_PROG", "0"}, {"HEART_COMPILE_FRAGMENT_PROG", "0"}, {"HEART_COMPILE_GEOMETRY_PROG", "0"}, {"HEART_COMPILE_COMPUTE_PROG", "0"}, {"HEART_COMPILE_HULL_PROG", "1"}, {"HEART_COMPILE_DOMAIN_PROG", "0"}, {"HEART_ES2", "0"}, {"HEART_WebGL", "0"}, {"HEART_ES3", "0"}, {"HEART_FL10", "0"}, {"HEART_FL11", "1"} },  Heart::proto::eShaderType_FL11_hs   , compileGLShader , Heart::proto::eShaderRenderSystem_OpenGL, },
    { "FL11_ds",      { {"HEART_COMPILE_VERTEX_PROG", "0"}, {"HEART_COMPILE_FRAGMENT_PROG", "0"}, {"HEART_COMPILE_GEOMETRY_PROG", "0"}, {"HEART_COMPILE_COMPUTE_PROG", "0"}, {"HEART_COMPILE_HULL_PROG", "0"}, {"HEART_COMPILE_DOMAIN_PROG", "1"}, {"HEART_ES2", "0"}, {"HEART_WebGL", "0"}, {"HEART_ES3", "0"}, {"HEART_FL10", "0"}, {"HEART_FL11", "1"} },  Heart::proto::eShaderType_FL11_ds   , compileGLShader , Heart::proto::eShaderRenderSystem_OpenGL, },
};

ShaderProfile getShaderProfile(const char* profile) {
    for (const auto& i : shaderProfiles) {
        if (strcmp(i.profileName, profile) == 0) {
            return i;
        }
    }
    fatal_error("profiles %s not supported!", profile);
    return shaderProfiles[0];
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

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
        case 'z': fprintf(stdout, "heart shader builder v0.8.1"); exit(0);
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

    std::vector<char> scratchbuffer;
    std::vector<ShaderProfile> profilesToCompile;
    std::vector<std::string> base_include_paths;
    ShaderCompileParams shader_compile_params;
    scratchbuffer.reserve(1024);
    const char* entry = "main";
    const char* profile = nullptr;
    shader_compile_params.entry_ = entry;
    shader_compile_params.compileFlags_ = 0;

    char root_path[MAX_PATH];
    minfs_path_parent(input_pb.resourceinputpath().c_str(), root_path, MAX_PATH);

    for (int i = 0, n = input_pb.buildparameters_size(); i < n; ++i) {
        auto& param = input_pb.buildparameters(i);
        if (param.name() == "profiles") {
            for (auto pi = 0; pi < param.values_size(); ++pi) {
                fatal_error_check(param.values(pi).has_strvalue(), "profiles must contain string values"); 
                profilesToCompile.push_back(getShaderProfile(param.values(pi).strvalue().c_str()));
            }
        } else if (param.name() == "include_dirs") {
            for (auto pi = 0; pi < param.values_size(); ++pi) {
                fatal_error_check(param.values(pi).has_strvalue(), "include_dirs must contain string values");
                char path_tmp[MAX_PATH];
                minfs_path_join(root_path, param.values(pi).strvalue().c_str(), path_tmp, MAX_PATH);
                base_include_paths.push_back(path_tmp);
            }
        } else if (param.name() == "debug" || param.name() == "skipoptimization" || param.name() == "warningsaserrors" || param.name() == "includesource") {
            // todo: handle
        } else {
            // We assume its a define of some kind
            fatal_error_check(param.values_size() == 1, "shader defines can't be arrays of parameters");
            if (param.values(0).has_strvalue()) {
                shader_compile_params.macros_.emplace_back(param.name().c_str(), param.values(0).strvalue().c_str());
            } else if (param.values(0).has_boolvalue()) {
                shader_compile_params.macros_.emplace_back(param.name().c_str(), param.values(0).boolvalue() ? "true" : "false");
            } else if (param.values(0).has_intvalue()) {
                char buf[4096];
                snprintf(buf, 4096, "%d", param.values(0).intvalue());
                shader_compile_params.macros_.emplace_back(param.name().c_str(), buf);
            } else if (param.values(0).has_floatvalue()) {
                char buf[4096];
                snprintf(buf, 4096, "%f", param.values(0).floatvalue());
                shader_compile_params.macros_.emplace_back(param.name().c_str(), buf);
            }
        }
    }

    proto::ShaderResourceContainer resource_container;
    std::string full_shader_source;
    std::map<std::string, std::string> parse_ctx;
    std::vector<std::string> included_files;
    parseShaderSource(input_pb.resourceinputpath().c_str(), base_include_paths, &full_shader_source, &parse_ctx, &included_files);

    for (const auto& i : profilesToCompile) {
        Heart::proto::ShaderResource* shaderresource = resource_container.add_shaderresources();
        std::string full_result_source = full_shader_source;
        std::string error_string;
        void* bin_blob;
        size_t bin_blob_len;
        auto local_shader_compile_params = shader_compile_params;
        for (const auto& d : i.defineString) {
            local_shader_compile_params.macros_.emplace_back(d.def, d.value);
        }
        local_shader_compile_params.profile_ = i.type;
        if (i.func_(&full_result_source, local_shader_compile_params, &error_string, &bin_blob, &bin_blob_len) != 0) {
            fatal_error("Shader Compile failed! Error Msg ::\n%s", error_string.c_str());
        }

        shaderresource->set_rendersystem(i.system_);
        shaderresource->set_entry(entry);
        shaderresource->set_profile(i.type);
        if (bin_blob) {
            shaderresource->set_compiledprogram(bin_blob, bin_blob_len);
        } else {
            shaderresource->set_source(full_result_source);
        }
        shaderresource->set_type(i.type);       
    }

    //write the resource
    Heart::builder::Output output;
    output.add_filedependency(input_pb.resourceinputpath());
    //write the resource header
    output.mutable_pkgdata()->set_type_name(resource_container.GetTypeName());
    output.mutable_pkgdata()->set_messagedata(resource_container.SerializeAsString());

    for (const auto& i : included_files) {
        output.add_filedependency(i);
    }

    google::protobuf::io::OstreamOutputStream filestream(&std::cout);
    return output.SerializeToZeroCopyStream(&filestream) ? 0 : -2;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

uint parseShaderSource(const std::string& shader_path, 
std::vector<std::string> in_include_paths, 
std::string* out_source_string,
std::map<std::string, std::string>* inc_ctx,
std::vector<std::string>* inc_files) {
    
    std::vector<char> scratch;
    scratch.reserve(1024);
    std::map<std::string, std::string> inc_map;
    scratch.resize(shader_path.length()+1);
    minfs_path_parent(shader_path.c_str(), scratch.data(), scratch.size());
    in_include_paths.insert(in_include_paths.begin(), scratch.data());
    out_source_string->clear();
    bool exist=minfs_is_file(shader_path.c_str()) != 0;
    if (!exist) {
        return -1;
    }
    size_t filesize=minfs_get_file_size(shader_path.c_str());
    char* buffer = new char[filesize+1];
    memset(buffer, 0, filesize+1);
    FILE* f = fopen(shader_path.c_str(), "rt");
    fread(buffer, 1, filesize, f);
    fclose(f);
    *out_source_string = buffer;
    // std::string line_dir = "#line 1\n";
    // out_source_string->insert(0, line_dir);
    delete[] buffer;
    buffer = nullptr;
    std::remove(out_source_string->begin(), out_source_string->end(), 0);
    // regex #includes in shader to be replaced later
    std::regex inc_regex("#include \\s*?\\\"(.*?)\\\"");
    sre_iterator re_i(out_source_string->begin(), out_source_string->end(), inc_regex);
    for (sre_iterator re_n; re_i != re_n; ++re_i) {
        std::string inc_string;
        bool did_include=true;
        for (auto i=in_include_paths.begin(), n=in_include_paths.end(); i!=n; ++i) {
            std::string inc_file = *i + "/" + (*re_i)[1].str();
            if (minfs_is_file(inc_file.c_str()) == 0) {
                continue;
            }
            inc_files->push_back(inc_file);
            const auto inc_source_itr = inc_ctx->find((*re_i)[1].str());
            if (inc_source_itr != inc_ctx->end()) {
                did_include = true;
                inc_map.insert(std::pair<std::string, std::string>((*re_i)[1].str(), inc_source_itr->second));
            } else if (parseShaderSource(inc_file, in_include_paths, &inc_string, inc_ctx, inc_files) == 0) {
                did_include = true;
                inc_ctx->insert(std::pair<std::string, std::string>((*re_i)[1].str(), inc_string));
                inc_map.insert(std::pair<std::string, std::string>((*re_i)[1].str(), inc_string));
            }
        }
        fatal_error_check(did_include, "Cannot include file %s", (*re_i)[1].str().c_str());
    }
    // replace #includes
    std::smatch re_match;
    while (std::regex_search(*out_source_string, re_match, inc_regex)) {
        const auto& replace_text = inc_map.find(re_match[1].str());
        out_source_string->replace(re_match.position(), re_match.length(), replace_text->second, 0, replace_text->second.length()-1);
    }

    return 0;
}

#if defined (PLATFORM_WINDOWS)
size_t parseVertexInputFormat(const D3D11_SHADER_DESC &desc, ID3D11ShaderReflection* reflect, Heart::proto::ShaderResource* shaderresource)
{
    size_t vertexInputLayoutFlags = 0;
    for ( size_t i = 0; i < desc.InputParameters; ++i )
    {
        Heart::proto::ShaderInputStream* stream=shaderresource->add_inputstreams();
        D3D11_SIGNATURE_PARAMETER_DESC inputDesc;
        reflect->GetInputParameterDesc( (UINT)i, &inputDesc );

        stream->set_semantic(inputDesc.SemanticName);
        stream->set_semanticindex(inputDesc.SemanticIndex);
        switch(inputDesc.Mask)
        {
        case 0x01: stream->set_elementcount(1); break;
        case 0x03: stream->set_elementcount(2); break;
        case 0x07: stream->set_elementcount(3); break;
        case 0x0F: stream->set_elementcount(4); break;
        default: return 0;
        }
        switch(inputDesc.ComponentType)
        {
        case D3D_REGISTER_COMPONENT_FLOAT32: stream->set_type(Heart::proto::eShaderStreamType_float); break;
        case D3D_REGISTER_COMPONENT_SINT32:  stream->set_type(Heart::proto::eShaderStreamType_sint32); break;
        case D3D_REGISTER_COMPONENT_UINT32:  stream->set_type(Heart::proto::eShaderStreamType_uint32); break;
        }
    }

    return desc.InputParameters;
}

uint compileD3DShader(std::string* shader_source, const ShaderCompileParams& shader_params, 
std::string* out_errors, void** bin_blob, size_t* bin_blob_len) {
    static const char* d3d_shaderProfiles[] = {
        "vs_4_0"        , //eShaderType_Vertex          = 0;
        "ps_4_0"        , //eShaderType_Pixel           = 1;
        "gs_4_0"        , //eShaderType_Geometery       = 2;
        "hs_5_0"        , //eShaderType_Hull            = 3;
        "ss_5_0"        , //eShaderType_Domain          = 4;
        "cs_4_0"        , //eShaderType_Compute         = 5;
        "vs_4_0"        , //eShaderType_vs4_0           = 6;
        "vs_4_1"        , //eShaderType_vs4_1           = 7;
        "vs_5_0"        , //eShaderType_vs5_0           = 8;
        "ps_4_0"        , //eShaderType_ps4_0           = 9;
        "ps_4_1"        , //eShaderType_ps4_1           = 10;
        "ps_5_0"        , //eShaderType_ps5_0           = 11;
        "gs_4_0"        , //eShaderType_gs4_0           = 12;
        "gs_4_1"        , //eShaderType_gs4_1           = 13;
        "gs_5_0"        , //eShaderType_gs5_0           = 14;
        "cs_4_0"        , //eShaderType_cs4_0           = 15;
        "cs_4_1"        , //eShaderType_cs4_1           = 16;
        "cs_5_0"        , //eShaderType_cs5_0           = 17;
        "hs_5_0"        , //eShaderType_hs5_0           = 18;
        "ds_5_0"        , //eShaderType_ds5_0           = 19;
    };
    struct D3DBlobSentry {
        D3DBlobSentry() : blob_(nullptr) {}
        ~D3DBlobSentry() { 
            if (blob_) {
                blob_->Release();
                blob_ = nullptr;
            }
        }
        ID3DBlob* blob_;
    };
    D3DBlobSentry errors;
    D3DBlobSentry result;

    std::string define_str;
    for (size_t i=0, n=shader_params.macros_.size(); i<n; ++i) {
        define_str = "#define " + shader_params.macros_[i].define_;
        define_str += " " + shader_params.macros_[i].value_ + "\n";
        shader_source->insert(0, define_str);
    }

    shader_source->insert(0, "#define HEART_IS_HLSL 1\n");

    //printf("D3D Source: \n%s\n", shader_source->c_str());

    HRESULT hr = D3DCompile( 
        shader_source->c_str(), 
        shader_source->length(), 
        "direct3D shader source",
        nullptr, // macros
        nullptr, //Includes
        shader_params.entry_.c_str(),
        d3d_shaderProfiles[shader_params.profile_], 
        shader_params.compileFlags_, 
        0, 
        &result.blob_, 
        &errors.blob_);

    if (FAILED(hr) && errors.blob_) {
        std::string err=(char*)errors.blob_->GetBufferPointer();
        *out_errors = (char*)errors.blob_->GetBufferPointer();
        return -1;
    }

    *bin_blob = new uchar[result.blob_->GetBufferSize()];
    *bin_blob_len = result.blob_->GetBufferSize();
    memcpy(*bin_blob, result.blob_->GetBufferPointer(), *bin_blob_len);

    return 0;
}
#endif // if defined (PLATFORM_WINDOWS)

uint initGLCompiler(std::string* out_errors){
    static bool run_once = false;
    if (run_once) {
        return 0;
    }
    
    SDL_Window *mainwindow;
    SDL_GLContext opengl_context;
 
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return -1; /* Or die on error */
    }
 
    mainwindow = SDL_CreateWindow("OpenGL Shader Compiler", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (!mainwindow) {
        return -2;
    }

    opengl_context = SDL_GL_CreateContext(mainwindow);

    glewExperimental=GL_TRUE;
    int ret = glewInit();
    if (ret != GLEW_OK) {
        *out_errors = (char*)glewGetErrorString(ret);
    }

    run_once = true;
    return 0;
}

FILE* makeTempFile(std::string* out_name) {
    char temp_path[MAX_PATH];
    
    if (GetTempPath(MAX_PATH, temp_path) == 0) {
        return nullptr;
    }
    char temp_name[MAX_PATH];
    char prefix[] = "ht_";
    FILE* out = nullptr;
    while (!out) {
        GetTempFileName(temp_path, prefix, 0, temp_name);
        out = fopen(temp_name, "wb");
    }

    *out_name = temp_name;
    return out;
}

uint compileGLShader(std::string* shader_source, const ShaderCompileParams& shader_params, 
std::string* out_errors, void** bin_blob, size_t* bin_blob_len) {
    struct GLSLProfile {
        const char* profileStr;
        GLuint type;
        const char* versionStr;
        const char* entry;
        const char* defineStr;
    };
    static const GLSLProfile OpenGL_shaderProfiles[] = {
        { "vs_4_0_level_9_3",   Heart::proto::eShaderType_ES2_vs,   "330"  , "main_vs", VS_COMMON_DEFINES },  
        { "ps_4_0_level_9_3",   Heart::proto::eShaderType_ES2_ps,   "330"  , "main_ps", PS_COMMON_DEFINES },
        { "vs_4_0_level_9_3",   Heart::proto::eShaderType_WebGL_vs, "es100", "main_vs", VS_COMMON_DEFINES },
        { "ps_4_0_level_9_3",   Heart::proto::eShaderType_WebGL_ps, "es100", "main_ps", PS_COMMON_DEFINES },
        { "vs_5_0"          ,   Heart::proto::eShaderType_ES3_vs,   "es300", "main_vs", VS_COMMON_DEFINES }, // version ??
        { "ps_5_0"          ,   Heart::proto::eShaderType_ES3_ps,   "es300", "main_ps", PS_COMMON_DEFINES }, // version ??
        { "vs_4_0"          ,   Heart::proto::eShaderType_FL10_vs,  "330"  , "main_vs", VS_COMMON_DEFINES }, // version ??
        { "ps_4_0"          ,   Heart::proto::eShaderType_FL10_ps,  "330"  , "main_ps", PS_COMMON_DEFINES }, // version ??
        { "gs_4_0"          ,   Heart::proto::eShaderType_FL10_gs,  "330"  , "main_gs", }, // version ??
        { "cs_4_0"          ,   Heart::proto::eShaderType_FL10_cs,  "330"  , "main_cs", }, // version ??
        { "vs_5_0"          ,   Heart::proto::eShaderType_FL11_vs,  "440"  , "main_vs", VS_COMMON_DEFINES }, // version ??
        { "ps_5_0"          ,   Heart::proto::eShaderType_FL11_ps,  "440"  , "main_ps", PS_COMMON_DEFINES }, // version ??
        { "gs_5_0"          ,   Heart::proto::eShaderType_FL11_gs,  "440"  , "main_gs", }, // version ??
        { "cs_5_0"          ,   Heart::proto::eShaderType_FL11_cs,  "440"  , "main_cs", }, // version ??
        { "hs_5_0"          ,   Heart::proto::eShaderType_FL11_hs,  "440"  , "main_hs", }, // version ??
        { "ds_5_0"          ,   Heart::proto::eShaderType_FL11_ds,  "440"  , "main_ds", }, // version ??
    };

    // no binaries for GL
    *bin_blob = nullptr;
    *bin_blob_len = 0;

    auto profile_index = shader_params.profile_-Heart::proto::eShaderType_MIN;

    std::string define_str;
    for (size_t i=0, n=shader_params.macros_.size(); i<n; ++i) {
        define_str = "#define " + shader_params.macros_[i].define_;
        define_str += " (" + shader_params.macros_[i].value_ + ")\n";
        shader_source->insert(0, define_str);
    }

    shader_source->insert(0, "#define HEART_IS_HLSL (1)\n");

    define_str = OpenGL_shaderProfiles[profile_index].defineStr;
    shader_source->insert(0, define_str);

    struct D3DBlobSentry {
        D3DBlobSentry() : blob_(nullptr) {}
        ~D3DBlobSentry() { 
            if (blob_) {
                blob_->Release();
                blob_ = nullptr;
            }
        }
        ID3DBlob* blob_;
    };
    D3DBlobSentry errors;
    D3DBlobSentry result;

    HRESULT hr = D3DCompile( 
        shader_source->c_str(), 
        shader_source->length(), 
        "direct3D shader source",
        nullptr, // macros
        nullptr, //Includes
        OpenGL_shaderProfiles[profile_index].entry,
        OpenGL_shaderProfiles[profile_index].profileStr, 
        shader_params.compileFlags_, 
        0, 
        &result.blob_, 
        &errors.blob_);

    if (FAILED(hr) && errors.blob_) {
        std::string err=(char*)errors.blob_->GetBufferPointer();
        *out_errors = (char*)errors.blob_->GetBufferPointer();
        return -1;
    }

    std::string tmp_filename;
    auto* tmp_file = makeTempFile(&tmp_filename);

    if (!tmp_file) {
        *out_errors = "Failed to create tempary file";
        return -1;
    }

    fwrite(result.blob_->GetBufferPointer(), result.blob_->GetBufferSize(), 1, tmp_file);
    fclose(tmp_file);

    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::string tmp_ref = tmp_filename+".json";
    std::string tmp_out = tmp_filename+".o";
    char cmdline[4096];
    snprintf(cmdline, sizeof(cmdline), "HLSLcc -lang=%s -in=%s -out=%s -reflect=%s -flags=%d", 
        OpenGL_shaderProfiles[profile_index].versionStr, tmp_filename.c_str(), tmp_out.c_str(), tmp_ref.c_str(), shader_params.compileFlags_);

    //fprintf(stderr, "HLSLcc command line: %s", cmdline);

    // Start the child process. 
    if (!CreateProcess(NULL,// No module name (use command line)
        cmdline,            // Command line
        NULL,               // Process handle not inheritable
        NULL,               // Thread handle not inheritable
        FALSE,              // Set handle inheritance to FALSE
        CREATE_NO_WINDOW,   // CREATE_NO_WINDOW prevents stdout,stderr redirection to this process
        NULL,               // Use parent's environment block
        NULL,               // Use parent's starting directory 
        &si,                // Pointer to STARTUPINFO structure
        &pi)               // Pointer to PROCESS_INFORMATION structure
        )
    {
        DeleteFile(tmp_filename.c_str());
        *out_errors = "Failed to call HLSLcc.";
        return -1;
    }

    DWORD exit_code;
    do {
        GetExitCodeProcess(pi.hProcess, &exit_code);
        Sleep(10);
    } while (exit_code == STILL_ACTIVE);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    bool failed = true;
    auto glsl_source_size = minfs_get_file_size(tmp_out.c_str());
    FILE* glsl_source_file = fopen(tmp_out.c_str(), "rb");
    if (glsl_source_file) {
        *bin_blob = new uint8_t[glsl_source_size];
        *bin_blob_len = glsl_source_size;
        fread(*bin_blob, 1, glsl_source_size, glsl_source_file);
        fclose(glsl_source_file);
        failed = false;
    }

    DeleteFile(tmp_filename.c_str());
    DeleteFile(tmp_ref.c_str());
    DeleteFile(tmp_out.c_str());
    return failed;
}

