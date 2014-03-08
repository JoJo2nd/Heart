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
#include <boost/filesystem.hpp>
#include <boost/smart_ptr.hpp>
#include <d3d11.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include "resource_shader.pb.h"

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


#define SB_API __cdecl
#if defined (shader_builder_EXPORTS)
#   define DLL_EXPORT __declspec(dllexport)
#else
#   define DLL_EXPORT __declspec(dllimport)
#endif

typedef unsigned char   uchar;
typedef unsigned int    uint;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

size_t parseVertexInputFormat(const D3D11_SHADER_DESC &desc, ID3D11ShaderReflection* reflect, Heart::proto::ShaderResource* shaderresource);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct FXIncludeHandler : public ID3DInclude 
{
    struct Include 
    {
        std::string                 includestring_; // string passed to #include pragma
        boost::filesystem::path     fullpath_;
        boost::filesystem::path     basepath_;
        std::shared_ptr<uchar>      data_;
        UINT                        datasize_;
    };
    typedef std::map<const void*, Include> IncludeMap;

    FXIncludeHandler() {}
    ~FXIncludeHandler() {}

    void addDefaultPath(const char* path) {
        Include inc;
        inc.fullpath_=path;
        inc.basepath_=boost::filesystem::canonical(inc.fullpath_.parent_path());
        inc.datasize_=0;
        includedFiles_.insert(IncludeMap::value_type((void*)(includedFiles_.size()), inc));
    }

    void addIncludePath(const char* path) {
        Include inc;
        inc.fullpath_=path;
        inc.basepath_=boost::filesystem::canonical(inc.fullpath_);
        inc.datasize_=0;
        includedFiles_.insert(IncludeMap::value_type((void*)(includedFiles_.size()), inc));
    }

    STDMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) {
        using namespace boost;

        system::error_code ec;

        for (IncludeMap::iterator i=includedFiles_.begin(), n=includedFiles_.end(); i!=n; ++i) {
            filesystem::path srcfile=i->second.basepath_ / filesystem::path(pFileName);
            bool exist=filesystem::exists(srcfile, ec);
            if (ec) {
                continue;
            }
            if (exist) {
                size_t filesize=filesystem::file_size(srcfile, ec);
                if (ec) {
                    continue;
                }
                std::shared_ptr<uchar> readbuffer(new uchar[filesize+1]);
                memset(readbuffer.get(), 0, filesize+1);
                FILE* f = fopen(srcfile.generic_string().c_str(), "rt");
                fread(readbuffer.get(), 1, filesize, f);
                fclose(f);
                Include inc;
                inc.includestring_=pFileName;
                inc.fullpath_=srcfile;
                inc.basepath_=inc.fullpath_.parent_path();
                inc.data_=readbuffer;
                inc.datasize_=(UINT)filesize;
                includedFiles_.insert(IncludeMap::value_type(readbuffer.get(), inc));

                *ppData = readbuffer.get();
                *pBytes = (UINT)filesize;
                return S_OK;
            }
        }
        return E_FAIL;
    }

    STDMETHOD(Close)(THIS_ LPCVOID pData) {
        return S_OK;
    }

    IncludeMap    includedFiles_;
};

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

const char* d3d_shaderProfiles[] = {
    "vs_4_0"   ,  //eShaderType_Vertex		= 0;
    "ps_4_0"    , //eShaderType_Pixel		= 1;
    "gs_4_0" ,     //eShaderType_Geometery	= 2;
    "hs_5_0"     ,//eShaderType_Hull		= 3;
    "ss_5_0"   ,  //eShaderType_Domain		= 4;
    "cs_4_0"  ,   //eShaderType_Compute		= 5;
    "vs_4_0"    , //eShaderType_vs4_0 = 6;
    "vs_4_1"    , //eShaderType_vs4_1 = 7;
    "vs_5_0"    , //eShaderType_vs5_0 = 8;
    "ps_4_0"    , //eShaderType_ps4_0 = 9;
    "ps_4_1"    , //eShaderType_ps4_1 = 10;
    "ps_5_0"    , //eShaderType_ps5_0 = 11;
    "gs_4_0"    , //eShaderType_gs4_0 = 12;
    "gs_4_1"    , //eShaderType_gs4_1 = 13;
    "gs_5_0"    , //eShaderType_gs5_0 = 14;
    "cs_4_0"    , //eShaderType_cs4_0 = 15;
    "cs_4_1"    , //eShaderType_cs4_1 = 16;
    "cs_5_0"    , //eShaderType_cs5_0 = 17;
    "hs_5_0"    , //eShaderType_hs5_0 = 18;
    "ds_5_0"    , //eShaderType_ds5_0 = 19;
};

struct ShaderDefine 
{
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

#define luaL_errorthrow(L, fmt, ...) \
    luaL_where(L, 1); \
    lua_pushfstring(L, fmt, __VA_ARGS__); \
    lua_concat(L, 2); \
    throw std::exception();

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int SB_API shaderCompiler(lua_State* L) {
    using namespace Heart;
    using namespace boost;
    /* Args from Lua (1: input files table, 2: dep files table, 3: parameter table, 4: outputpath)*/
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TTABLE);
    luaL_checktype(L, 3, LUA_TTABLE);
    luaL_checktype(L, 4, LUA_TSTRING);
    
try {
    system::error_code ec;
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

    Heart::proto::ShaderResource shaderresource;
    FXIncludeHandler includeHandler;
    HRESULT hr;
    bool includesource=false;
    D3DBlobSentry errors;
    D3DBlobSentry result;
    uint compileFlags = 0;
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
    lua_getfield(L, 3, "includesource");
    if (lua_toboolean(L, -1)) {
        includesource=true;
    }
    lua_pop(L, 1);

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
    D3D_SHADER_MACRO macro;
    std::vector<D3D_SHADER_MACRO> fullmacros;
    fullmacros.reserve(64);
    macro.Name = "HEART_USING_HLSL"; macro.Definition = "1";
    fullmacros.push_back(macro);
    macro.Name = "HEART_ENGINE"; macro.Definition = "1";
    fullmacros.push_back(macro);
    macro.Name = progTypeMacros[progtype]; macro.Definition = "1";
    fullmacros.push_back(macro);

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
            macro.Name = lua_tostring(L,-2);
            macro.Definition = lua_tostring(L, -1);
            fullmacros.emplace_back(macro);
            /* removes 'value'; keeps 'key' for next iteration */
            lua_pop(L, 1);
            ++i;
        }
    }
    lua_pop(L, 1);
    macro.Name = nullptr;
    macro.Definition = nullptr;
    fullmacros.push_back(macro);

    const char* path=lua_tostring(L, 1);
    includeHandler.addDefaultPath(path);

    lua_getfield(L, 3, "include_dirs");
    if (lua_istable(L, -1)) {
        auto source_root = filesystem::path(path).parent_path();
        lua_pushnil(L);
        while (lua_next(L, -2) != 0) {
            /* uses 'key' (at index -2) and 'value' (at index -1) */
            if (lua_isstring(L, -1)) {
                auto include_path = source_root / lua_tostring(L, -1);
                auto include_path_str = include_path.generic_string();
                includeHandler.addIncludePath(include_path_str.c_str());
            }
            /* removes 'value'; keeps 'key' for next iteration */
            lua_pop(L, 1);
        }
    }

    size_t filesize=(size_t)filesystem::file_size(path, ec);
    if (ec) {
        luaL_errorthrow(L, "Unable to read filesize of shader input %s", path);
    }
    std::shared_ptr<char> sourcedata;
    size_t sourcedatalen = filesize;
    sourcedata = std::shared_ptr<char>(new char[sourcedatalen+1]);
    memset(sourcedata.get(), 0, sourcedatalen);
    FILE* f=fopen(path, "rt");
    if (!f) {
        luaL_errorthrow(L, "Unable to open shader input file %s", path);
    }
    fread(sourcedata.get(), 1, sourcedatalen, f);
    fclose(f);
    sourcedata.get()[sourcedatalen] = 0;
    hr = D3DCompile( 
        sourcedata.get(), 
        sourcedatalen, 
        path,
        fullmacros.data(), 
        &includeHandler, //Includes
        entry,
        d3d_shaderProfiles[progtype], 
        compileFlags, 
        0, 
        &result.blob_, 
        &errors.blob_);

    if (FAILED(hr) && errors.blob_) {
        std::string err=(char*)errors.blob_->GetBufferPointer();
        luaL_errorthrow(L, "Shader Compile failed! Error Msg ::\n%s", (char*)errors.blob_->GetBufferPointer());
    }

    shaderresource.set_entry(entry);
    shaderresource.set_profile(progtype);
    shaderresource.set_compiledprogram(result.blob_->GetBufferPointer(), result.blob_->GetBufferSize());
    shaderresource.set_type(progtype);
    for (size_t i=0, n=fullmacros.size()-1; i<n; ++i) {
        proto::ShaderResource_Defines* defres=shaderresource.add_defines();
        defres->set_define(fullmacros[i].Name);
        defres->set_value(fullmacros[i].Definition);
    }

    ID3D11ShaderReflection* reflect;
    hr = D3DReflect(result.blob_->GetBufferPointer(), result.blob_->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflect);
    if (FAILED( hr )) {
        luaL_errorthrow(L, "Couldn't create reflection information.");
    }

    D3D11_SHADER_DESC desc;
    reflect->GetDesc( &desc );
    if (progtype == proto::eShaderType_Vertex) {
        if (parseVertexInputFormat(desc, reflect, &shaderresource) == 0) {
            luaL_errorthrow(L, "Failed to parse vertex input format from shader");
        }
    }

    //output the source sections desc
    if (includesource) {
        shaderresource.set_source(sourcedata.get(), sourcedatalen);
        for (auto i=includeHandler.includedFiles_.begin(), n=includeHandler.includedFiles_.end(); i!=n; ++i) {
            if (i->second.datasize_ > 0) {
                Heart::proto::ShaderIncludeSource* include = shaderresource.add_includedfiles();
                include->set_filepath(i->second.fullpath_.generic_string());
                include->set_source((char*)i->second.data_.get(), i->second.datasize_);
                include->set_filepath_short(i->second.includestring_);
            }
        }
    }

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
        msgContainer.set_type_name(shaderresource.GetTypeName());
        msgContainer.set_messagedata(shaderresource.SerializeAsString());
        msgContainer.SerializePartialToCodedStream(&outputstream);
    }
    output.close();

    lua_newtable(L); // push table of files files that where included by 
    int idx=1;
    for (FXIncludeHandler::IncludeMap::iterator i=includeHandler.includedFiles_.begin(), n=includeHandler.includedFiles_.end(); i!=n; ++i) {
        lua_pushstring(L, i->second.fullpath_.generic_string().c_str());
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
