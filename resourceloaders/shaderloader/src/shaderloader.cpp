/********************************************************************

    filename: 	shaderloader.cpp	
    
    Copyright (c) 31:7:2012 James Moran
    
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
#include "resource_shader.pb.h"
#include "minfs.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
};

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

#if defined PLATFORM_WINDOWS
#   define SB_API __cdecl
#elif PLATFORM_LINUX
#   if BUILD_64_BIT
#       define SB_API
#   else
#       define SB_API __attribute__((cdecl))
#   endif
#else
#   error
#endif

#if defined (PLATFORM_WINDOWS)
#   if defined (shader_builder_EXPORTS)
#       define DLL_EXPORT __declspec(dllexport)
#   else
#       define DLL_EXPORT __declspec(dllimport)
#   endif
#else
#   define DLL_EXPORT
#endif

typedef unsigned char   uchar;
typedef unsigned int    uint;
typedef std::regex_iterator<std::string::iterator> sre_iterator;

#define luaL_errorthrow(L, fmt, ...) \
    luaL_where(L, 1); \
    lua_pushfstring(L, fmt, ##__VA_ARGS__ ); \
    lua_concat(L, 2); \
    throw std::exception();


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const char* shaderProfiles[] = {
    "vertex"   ,  //eShaderType_Vertex		= 0;
    "pixel"    , //eShaderType_Pixel		= 1;
    "geometry" ,     //eShaderType_Geometery	= 2;
    "hull"     ,//eShaderType_Hull		= 3;
    "domain"   ,  //eShaderType_Domain		= 4;
    "compute"  ,   //eShaderType_Compute		= 5;
    "vs4_0"    , //eShaderType_vs4_0 = 6;
    "vs4_1"    , //eShaderType_vs4_1 = 7;
    "vs5_0"    , //eShaderType_vs5_0 = 8;
    "ps4_0"    , //eShaderType_ps4_0 = 9;
    "ps4_1"    , //eShaderType_ps4_1 = 10;
    "ps5_0"    , //eShaderType_ps5_0 = 11;
    "gs4_0"    , //eShaderType_gs4_0 = 12;
    "gs4_1"    , //eShaderType_gs4_1 = 13;
    "gs5_0"    , //eShaderType_gs5_0 = 14;
    "cs4_0"    , //eShaderType_cs4_0 = 15;
    "cs4_1"    , //eShaderType_cs4_1 = 16;
    "cs5_0"    , //eShaderType_cs5_0 = 17;
    "hs5_0"    , //eShaderType_hs5_0 = 18;
    "ds5_0"    , //eShaderType_ds5_0 = 19;
};


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
std::string* out_source_string, std::map<std::string, std::string>* inc_ctx);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int SB_API shaderCompiler(lua_State* L) {
    using namespace Heart;

    /* Args from Lua (1: input files table, 2: dep files table, 3: parameter table, 4: outputpath)*/
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TTABLE);
    luaL_checktype(L, 3, LUA_TTABLE);
    luaL_checktype(L, 4, LUA_TSTRING);

try {
    std::vector<char> scratchbuffer;
    scratchbuffer.reserve(1024);
    proto::eShaderType progtype;
    const char* entry = nullptr;
    const char* profile = nullptr;
    lua_getfield(L, 3, "entry");
    if (!lua_isstring(L, -1)) {
        luaL_errorthrow(L, "entry parameter is not a string or missing");
    }
    entry=lua_tostring(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, 3, "profile");
    progtype = (proto::eShaderType)luaL_checkoption(L, -1, "vs4_0", shaderProfiles);
    lua_pop(L, 1);

    ShaderCompileParams shader_compile_params;
    uint compileFlags = 0;
#if defined (PLATFORM_WINDOWS)    
    lua_getfield(L, 3, "debug");
    if (lua_toboolean(L, -1)) {
        compileFlags |= D3DCOMPILE_DEBUG;
    }
    lua_pop(L, 1);
    lua_getfield(L, 3, "skipoptimization");
    if (lua_toboolean(L, -1)) {
        compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
    }
    lua_pop(L, 1);
    lua_getfield(L, 3, "warningsaserrors");
    if (lua_toboolean(L, -1)) {
        compileFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
    }
    lua_pop(L, 1);
#endif
    uint definecount=0;
    bool hasdefines=false;
    lua_getfield(L, 3, "defines");
    if (!lua_istable(L, -1) && !lua_isnil(L, -1)) {
        luaL_errorthrow(L, "defines unexpected type (got %s; expected %s)", lua_typename(L, lua_type(L, -1)), lua_typename(L, LUA_TTABLE));
    }
    if (lua_istable(L, -1)) {
        hasdefines=true;
    }

    const char* progTypeMacros[] = {
        "HEART_COMPILE_VERTEX_PROG"  ,    // "vertex"    
        "HEART_COMPILE_FRAGMENT_PROG",    // "pixel"     
        "HEART_COMPILE_GEOMETRY_PROG",    // "geometery" 
        "HEART_COMPILE_HULL_PROG"    ,    // "hull"      
        "HEART_COMPILE_DOMAIN_PROG"  ,    // "domain"    
        "HEART_COMPILE_COMPUTE_PROG" ,    // "compute"   
        "HEART_COMPILE_VERTEX_PROG"  ,    // "vs4_0"     
        "HEART_COMPILE_VERTEX_PROG"  ,    // "vs4_1"     
        "HEART_COMPILE_VERTEX_PROG"  ,    // "vs5_0"     
        "HEART_COMPILE_FRAGMENT_PROG",    // "ps4_0"     
        "HEART_COMPILE_FRAGMENT_PROG",    // "ps4_1"     
        "HEART_COMPILE_FRAGMENT_PROG",    // "ps5_0"     
        "HEART_COMPILE_GEOMETRY_PROG",    // "gs4_0"     
        "HEART_COMPILE_GEOMETRY_PROG",    // "gs4_1"     
        "HEART_COMPILE_GEOMETRY_PROG",    // "gs5_0"     
        "HEART_COMPILE_COMPUTE_PROG" ,    // "cs4_0"     
        "HEART_COMPILE_COMPUTE_PROG" ,    // "cs4_1"     
        "HEART_COMPILE_COMPUTE_PROG" ,    // "cs5_0"     
        "HEART_COMPILE_HULL_PROG"    ,    // "hs5_0"     
        "HEART_COMPILE_DOMIAN_PROG"  ,    // "ds5_0"     
    };

    shader_compile_params.entry_ = entry;
    shader_compile_params.compileFlags_ = compileFlags;
    shader_compile_params.profile_ = progtype;
    shader_compile_params.macros_.emplace_back("HEART_ENGINE", "1");
    shader_compile_params.macros_.emplace_back(progTypeMacros[progtype], "1");

    if (hasdefines) {
        uint i=0;
        lua_pushnil(L);
        while (lua_next(L, -2) != 0) {
            /* uses 'key' (at index -2) and 'value' (at index -1) */
            if (!lua_isstring(L,-2) && !lua_isstring(L,-1)) {
                luaL_errorthrow(L, "DEFINE table entry unexpected type(got [%s]=%s; expected [%s]=%s)", 
                    lua_typename(L, lua_type(L, -2)), lua_typename(L, lua_type(L, -1)), lua_typename(L, LUA_TSTRING), lua_typename(L, LUA_TSTRING));
                break;
            }
            shader_compile_params.macros_.emplace_back(lua_tostring(L,-2), lua_tostring(L,-1));
            /* removes 'value'; keeps 'key' for next iteration */
            lua_pop(L, 1);
            ++i;
        }
    }
    lua_pop(L, 1);

    const char* path=lua_tostring(L, 1);
    std::vector<std::string> base_include_paths;

    lua_getfield(L, 3, "include_dirs");
    if (lua_istable(L, -1)) {
        size_t pathlen=strlen(path)+1;
        scratchbuffer.resize(strlen(path)+1);
        minfs_path_parent(path, scratchbuffer.data(), pathlen);
        std::string source_root = scratchbuffer.data();
        lua_pushnil(L);
        while (lua_next(L, -2) != 0) {
            /* uses 'key' (at index -2) and 'value' (at index -1) */
            if (lua_isstring(L, -1)) {
                base_include_paths.push_back(source_root + "/" + lua_tostring(L, -1));
            }
            /* removes 'value'; keeps 'key' for next iteration */
            lua_pop(L, 1);
        }
    }

    struct ShaderCompiler {
        CompilerFunc func_;
        proto::eShaderRenderSystem system_;
    };
    ShaderCompiler compilers[] = {
#if defined (PLATFORM_WINDOWS)
//        {compileD3DShader, proto::eShaderRenderSystem_D3D11},
#endif
        {compileGLShader , proto::eShaderRenderSystem_OpenGL},
        {nullptr, proto::eShaderRenderSystem_None}
    };

    proto::ShaderResourceContainer resource_container;
    std::string full_shader_source;
    std::map<std::string, std::string> parse_ctx;
    parseShaderSource(path, base_include_paths, &full_shader_source, &parse_ctx);

    ShaderCompiler* current_compiler = compilers;
    while (current_compiler->func_){
        Heart::proto::ShaderResource* shaderresource = resource_container.add_shaderresources();
        std::string full_result_source = full_shader_source;
        std::string error_string;
        void* bin_blob;
        size_t bin_blob_len;
        if (current_compiler->func_(&full_result_source, shader_compile_params, &error_string, &bin_blob, &bin_blob_len) != 0) {
            luaL_errorthrow(L, "Shader Compile failed! Error Msg ::\n%s", error_string.c_str());
        }

        shaderresource->set_rendersystem(current_compiler->system_);
        shaderresource->set_entry(entry);
        shaderresource->set_profile(progtype);
        if (bin_blob) {
            shaderresource->set_compiledprogram(bin_blob, bin_blob_len);
        }
        shaderresource->set_type(progtype);
        shaderresource->set_source(full_result_source);

        ++current_compiler;
    }

    // ID3D11ShaderReflection* reflect;
    // hr = D3DReflect(result.blob_->GetBufferPointer(), result.blob_->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflect);
    // if (FAILED( hr )) {
    //     luaL_errorthrow(L, "Couldn't create reflection information.");
    // }

    // D3D11_SHADER_DESC desc;
    // reflect->GetDesc( &desc );
    // if (progtype == proto::eShaderType_Vertex) {
    //     if (parseVertexInputFormat(desc, reflect, &shaderresource) == 0) {
    //         luaL_errorthrow(L, "Failed to parse vertex input format from shader");
    //     }
    // }

    //write the resource
    const char* outputpath=lua_tostring(L, 4);
    std::ofstream output;
    output.open(outputpath, std::ios_base::out|std::ios_base::binary);
    if (!output.is_open()) {
        luaL_errorthrow(L, "Unable to open output file %s", outputpath);
    }

    google::protobuf::io::OstreamOutputStream filestream(&output);
    google::protobuf::io::CodedOutputStream outputstream(&filestream);
    {
        google::protobuf::io::OstreamOutputStream filestream(&output);
        google::protobuf::io::CodedOutputStream outputstream(&filestream);
        Heart::proto::MessageContainer msgContainer;
        msgContainer.set_type_name(resource_container.GetTypeName());
        msgContainer.set_messagedata(resource_container.SerializeAsString());
        msgContainer.SerializePartialToCodedStream(&outputstream);
    }
    output.close();

    lua_newtable(L); // push table of files files that where included by the shader (parse_ctx should have this info)
    int idx=1;
    for (auto i=parse_ctx.begin(), n=parse_ctx.end(); i!=n; ++i) {
        lua_pushstring(L, i->first.c_str());
        lua_rawseti(L, -2, idx);
        ++idx;
    }

    return 1;
} catch (std::exception e) {
    return lua_error(L);
}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

uint parseShaderSource(const std::string& shader_path, 
std::vector<std::string> in_include_paths, 
std::string* out_source_string,
std::map<std::string, std::string>* inc_ctx) {
    
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
            const auto inc_source_itr = inc_ctx->find((*re_i)[1].str());
            if (inc_source_itr != inc_ctx->end()) {
                did_include = true;
                inc_map.insert(std::pair<std::string, std::string>((*re_i)[1].str(), inc_source_itr->second));
            } else if (parseShaderSource(inc_file, in_include_paths, &inc_string, inc_ctx) == 0) {
                did_include = true;
                inc_ctx->insert(std::pair<std::string, std::string>((*re_i)[1].str(), inc_string));
                inc_map.insert(std::pair<std::string, std::string>((*re_i)[1].str(), inc_string));
            }
        }
        if (!did_include) {
            //todo: throw error!
        }
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

    printf("D3D Source: \n%s\n", shader_source->c_str());

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
 
    // Request opengl 3.3 context.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
 
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

uint compileGLShader(std::string* shader_source, const ShaderCompileParams& shader_params, 
std::string* out_errors, void** bin_blob, size_t* bin_blob_len) {
    if (initGLCompiler(out_errors) != 0) {
        *out_errors = "Failed to initialise OpenGL";
    }
    static const GLuint OpenGL_shaderProfiles[] = {
        GL_VERTEX_SHADER    , //eShaderType_Vertex          = 0;
        GL_FRAGMENT_SHADER  , //eShaderType_Pixel           = 1;
        GL_GEOMETRY_SHADER  , //eShaderType_Geometery       = 2;
        GL_INVALID_ENUM     , //eShaderType_Hull            = 3;
        GL_INVALID_ENUM     , //eShaderType_Domain          = 4;
        GL_INVALID_ENUM     , //eShaderType_Compute         = 5;
        GL_VERTEX_SHADER    , //eShaderType_vs4_0           = 6;
        GL_VERTEX_SHADER    , //eShaderType_vs4_1           = 7;
        GL_VERTEX_SHADER    , //eShaderType_vs5_0           = 8;
        GL_FRAGMENT_SHADER  , //eShaderType_ps4_0           = 9;
        GL_FRAGMENT_SHADER  , //eShaderType_ps4_1           = 10;
        GL_FRAGMENT_SHADER  , //eShaderType_ps5_0           = 11;
        GL_GEOMETRY_SHADER  , //eShaderType_gs4_0           = 12;
        GL_GEOMETRY_SHADER  , //eShaderType_gs4_1           = 13;
        GL_GEOMETRY_SHADER  , //eShaderType_gs5_0           = 14;
        GL_INVALID_ENUM     , //eShaderType_cs4_0           = 15;
        GL_INVALID_ENUM     , //eShaderType_cs4_1           = 16;
        GL_INVALID_ENUM     , //eShaderType_cs5_0           = 17;
        GL_INVALID_ENUM     , //eShaderType_hs5_0           = 18;
        GL_INVALID_ENUM     , //eShaderType_ds5_0           = 19;
    };

    // no binaries for GL
    *bin_blob = nullptr;
    *bin_blob_len = 0;

    GLuint shader_type = OpenGL_shaderProfiles[shader_params.profile_];

    std::string define_str;
    for (size_t i=0, n=shader_params.macros_.size(); i<n; ++i) {
        define_str = "#define " + shader_params.macros_[i].define_;
        define_str += " " + shader_params.macros_[i].value_ + "\n";
        shader_source->insert(0, define_str);
    }

    shader_source->insert(0, "#define HEART_IS_GLSL 1\n");

    define_str = "#version 330\n";
    shader_source->insert(0, define_str);

    // parse the gl shader looking for input & output
    // maybe at somepoint I'll fix this to auto-gen boiler plate for this...
    std::string input_struct_name;
    std::string output_struct_name;
    std::regex in_out_regex("glsl_(in|out)_struct\\(\\s*(.+?)\\s*\\)");
    sre_iterator re_i(shader_source->begin(), shader_source->end(), in_out_regex);
    for (sre_iterator re_n; re_i!=re_n; ++re_i) {
        if ((*re_i)[2].str() == "__x__") {
            continue;
        }
        if ((*re_i)[1].str() == "in") {
            input_struct_name = (*re_i)[2].str();
        } else if ((*re_i)[1].str() == "out") {
            output_struct_name = (*re_i)[2].str();
        }
    }

    printf("input struct name: %s\n", input_struct_name.c_str());
    printf("output struct name: %s\n", output_struct_name.c_str());

    //read in the parameters for input (seperate function later...?)
    //std::string tmp_str("struct\\s*?");
    //std::regex in_struct_regex(tmp_str+input_struct_name+)

    printf("OpenGL Source: \n%s\n", shader_source->c_str());

    GLint params = -1;
    GLuint shader_obj = glCreateShader(shader_type);
    const char* sources[] = {
        shader_source->c_str(),
    };
    GLint sources_len[] = {
        (GLint)shader_source->length(),
    };
    glShaderSource(shader_obj, 1, sources, sources_len);
    glCompileShader(shader_obj);
    glGetShaderiv(shader_obj, GL_COMPILE_STATUS, &params);
    if (params != GL_TRUE) {
        GLint actual_length = 0;
        char log[8*1024] = {0};
        glGetShaderInfoLog (shader_obj, sizeof(log)-1, &actual_length, log);
        *out_errors = log;
        return -1;
    }
    glDeleteShader(shader_obj);
    return 0;
}

extern "C" {
//Lua entry point calls
DLL_EXPORT int SB_API luaopen_gpuprogram(lua_State *L) {
    static const luaL_Reg gpuproglib[] = {
        {"build",shaderCompiler},
        {NULL, NULL}
    };

    luaL_newlib(L, gpuproglib);

    return 1;
}
};
