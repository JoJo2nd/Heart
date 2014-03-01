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

#include "heart.h"
#include <d3d11.h>
#include <boost/filesystem.hpp>
#include <boost/smart_ptr.hpp>
#include <string>
#include <map>

#include <stdio.h>
#include <vector>

#define SB_API __cdecl
#if defined (shader_builder_EXPORTS)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 ParseVertexInputFormat(const D3D11_SHADER_DESC &desc, ID3D11ShaderReflection* reflect, Heart::proto::ShaderResource* shaderresource);

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
        boost::shared_ptr<hByte>    data_;
        UINT                        datasize_;
    };
    typedef std::map<void*, Include> IncludeMap;

    FXIncludeHandler() {}
    ~FXIncludeHandler() {}

    void addDefaultPath(const hChar* path) {
        Include inc;
        inc.fullpath_=path;
        inc.basepath_=inc.fullpath_.parent_path();
        inc.datasize_=0;
        includedFiles_.insert(IncludeMap::value_type(hNullptr, inc));
    }

    STDMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) {
        using namespace Heart;
        using namespace boost;

        system::error_code ec;

        for (IncludeMap::iterator i=includedFiles_.begin(), n=includedFiles_.end(); i!=n; ++i) {
            filesystem::path srcfile=i->second.basepath_ / filesystem::path(pFileName);
            bool exist=filesystem::exists(srcfile, ec);
            if (ec) {
                continue;
            }
            if (exist) {
                hUint filesize=(hUint)filesystem::file_size(srcfile, ec);
                if (ec) {
                    continue;
                }
                shared_ptr<hByte> readbuffer(new hByte[filesize+1]);
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

Heart::hShaderProfile getProfileFromString(const hChar* str) {
    static const hChar* s_shaderProfileNames[] = {
        "vs_4_0",   //eShaderProfile_vs4_0,
        "vs_4_1",   //eShaderProfile_vs4_1,
        "vs_5_0",   //eShaderProfile_vs5_0,
        //
        "ps_4_0",   //eShaderProfile_ps4_0,
        "ps_4_1",   //eShaderProfile_ps4_1,
        "ps_5_0",   //eShaderProfile_ps5_0,
        //
        "gs_4_0",   //eShaderProfile_gs4_0,
        "gs_4_1",   //eShaderProfile_gs4_1,
        "gs_5_0",   //eShaderProfile_gs5_0,
        //
        "cs_4_0",   //eShaderProfile_cs4_0,
        "cs_4_1",   //eShaderProfile_cs4_1,
        "cs_5_0",   //eShaderProfile_cs5_0,
        //
        "hs_5_0",   //eShaderProfile_hs5_0,
        "ds_5_0",   //eShaderProfile_ds5_0,
    };

    for (hUint i=0; i<hStaticArraySize(s_shaderProfileNames); ++i) {
        if (_stricmp(s_shaderProfileNames[i],str) == 0) {
            return (Heart::hShaderProfile)i;
        }
    }
    return Heart::eShaderProfile_Max;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int SB_API shaderCompiler(lua_State* L) {
    using namespace Heart;
    using namespace boost;
    /* Args from Lua (1: input files table, 2: dep files table, 3: parameter table, 4: outputpath)*/
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TTABLE);
    luaL_checktype(L, 3, LUA_TTABLE);
    luaL_checktype(L, 4, LUA_TSTRING);
    
    system::error_code ec;
    proto::eShaderType progtype;
    const char* entry = hNullptr;/*params->GetBuildParameter("ENTRY","main")*/;
    const char* profile = hNullptr;//params->GetBuildParameter("PROFILE","vs_5_0");
    lua_getfield(L, 3, "entry");
    if (!lua_isstring(L, -1)) {
        luaL_error(L, "entry parameter is not a string or missing");
        return 0;
    }
    entry=lua_tostring(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, 3, "profile");
    if (!lua_isstring(L, -1)) {
        luaL_error(L, "profile parameter is not a string or missing");
        return 0;
    }
    profile=lua_tostring(L, -1);
    lua_pop(L, 1);

    hShaderProfile profileType=getProfileFromString(profile);
    if (profileType >= eShaderProfile_vs4_0 && profileType <= eShaderProfile_vs5_0) {
        progtype = proto::eShaderType_Vertex;
    } else if (profileType >= eShaderProfile_ps4_0 && profileType <= eShaderProfile_ps5_0) {
        progtype = proto::eShaderType_Pixel;
    } else if (profileType >= eShaderProfile_gs4_0 && profileType <= eShaderProfile_gs5_0) {
        progtype = proto::eShaderType_Geometery;
    } else if (profileType >= eShaderProfile_cs4_0 && profileType <= eShaderProfile_cs5_0) {
        progtype = proto::eShaderType_Compute;
    } else if (profileType == eShaderProfile_hs5_0) {
        progtype = proto::eShaderType_Hull;    
    } else if (profileType == eShaderProfile_ds5_0) {
        progtype = proto::eShaderType_Domain;
    } else {
        luaL_error(L, "Unable to build GPU profile %s", profile);
        return 0;
    }

    Heart::proto::ShaderResource shaderresource;
    FXIncludeHandler includeHandler;
    HRESULT hr;
    hBool includesource=hFalse;
    ID3DBlob* errors;
    ID3DBlob* result;
    hUint32 compileFlags = 0;
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
        includesource=hTrue;
    }
    lua_pop(L, 1);

    hUint definecount=0;
    hBool hasdefines=hFalse;
    lua_getfield(L, 3, "defines");
    if (!lua_istable(L, -1) && !lua_isnil(L, -1)) {
        luaL_error(L, "defines unexpected type (got %s; expected %s)", lua_typename(L, lua_type(L, -1)), lua_typename(L, LUA_TTABLE));
    }
    if (lua_istable(L, -1)) {
        hasdefines=hTrue;
    }
    const hChar* progTypeMacros[] = {
        "HEART_COMPILE_VERTEX_PROG"  ,
        "HEART_COMPILE_FRAGMENT_PROG",
        "HEART_COMPILE_GEOMETRY_PROG",
        "HEART_COMPILE_HULL_PROG"    ,
        "HEART_COMPILE_DOMAIN_PROG"  ,
        "HEART_COMPILE_COMPUTE_PROG" ,
        "HEART_COMPILE_UNKNOWN"      ,
    };
    D3D_SHADER_MACRO defaultmacros[] =
    {
        { "HEART_USING_HLSL", "1" },
        { "HEART_ENGINE", "1" },
        { progTypeMacros[progtype], "1" },
        { NULL, NULL }
    };

    D3D_SHADER_MACRO* fullmacros=hNullptr;
    definecount=0;
    if (hasdefines) {
        lua_pushnil(L);
        while (lua_next(L, -2) != 0) {
            ++definecount;
            /* removes 'value'; keeps 'key' for next iteration */
            lua_pop(L, 1);
        }
    }
    fullmacros=(D3D_SHADER_MACRO*)hAlloca(sizeof(D3D_SHADER_MACRO)*(definecount+1+hStaticArraySize(defaultmacros)));
    if (hasdefines) {
        hUint i=0;
        lua_pushnil(L);
        while (lua_next(L, -2) != 0) {
            /* uses 'key' (at index -2) and 'value' (at index -1) */
            if (!lua_isstring(L,-2) && !lua_isstring(L,-1)) {
                luaL_error(L, "DEFINE table entry unexpected type(got [%s]=%s; expected [%s]=%s)", 
                    lua_typename(L, lua_type(L, -2)), lua_typename(L, lua_type(L, -1)), lua_typename(L, LUA_TSTRING), lua_typename(L, LUA_TSTRING));
                return 0;
            }
            fullmacros[i].Name=lua_tostring(L,-2);
            fullmacros[i].Definition=lua_tostring(L, -1);
            /* removes 'value'; keeps 'key' for next iteration */
            lua_pop(L, 1);
            ++i;
        }
    }
    memcpy(fullmacros+definecount, defaultmacros, sizeof(defaultmacros));
    lua_pop(L, 1);

    lua_rawgeti(L, 1, 1);
    if (!lua_isstring(L, -1)) {
        luaL_error(L, "input file is not a string");
        return 0;
    }
    lua_getglobal(L, "buildpathresolve");
    lua_pushvalue(L, -2);
    lua_call(L, 1, 1);
    const hChar* path=lua_tostring(L, -1);
    includeHandler.addDefaultPath(path);
    hUint filesize=(hUint)filesystem::file_size(path, ec);
    if (ec) {
        luaL_error(L, "Unable to read filesize of shader input %s", path);
        return 0;
    }
    shared_array<hChar> sourcedata;
    hUint sourcedatalen = filesize;
    sourcedata = shared_array<hChar>(new hChar[sourcedatalen+1]);
    memset(sourcedata.get(), 0, sourcedatalen);
    FILE* f=fopen(path, "rt");
    if (!f) {
        luaL_error(L, "Unable to open shader input file %s", path);
        return 0;
    }
    fread(sourcedata.get(), 1, sourcedatalen, f);
    fclose(f);
    sourcedata[sourcedatalen] = 0;
    hr = D3DCompile( 
        sourcedata.get(), 
        sourcedatalen, 
        path,
        fullmacros, 
        &includeHandler, //Includes
        entry,
        profile, 
        compileFlags, 
        0, 
        &result, 
        &errors);
    lua_pop(L, 1);

    if (FAILED(hr) && errors) {
        std::string err=(hChar*)errors->GetBufferPointer();
        errors->Release();
        errors = NULL;
        luaL_error(L, "Shader Compile failed! Error Msg ::\n%s", err.c_str());
    } else if (errors) {
        lua_getglobal(L, "print");
        lua_pushstring(L, "Shader Compile output:\n");
        lua_pushstring(L, (hChar*)errors->GetBufferPointer());
        errors->Release();
        errors = NULL;
        lua_call(L, 2, 0);
    }

    shaderresource.set_entry(entry);
    shaderresource.set_profile(getProfileFromString(profile));
    shaderresource.set_compiledprogram(result->GetBufferPointer(), result->GetBufferSize());
    shaderresource.set_type(progtype);
    for (hUint i=0; fullmacros[i].Name; ++i) {
        proto::ShaderResource_Defines* defres=shaderresource.add_defines();
        defres->set_define(fullmacros[i].Name);
        defres->set_value(fullmacros[i].Definition);
    }

    ID3D11ShaderReflection* reflect;
    hr = D3DReflect(result->GetBufferPointer(), result->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflect);
    if (FAILED( hr )) {
        luaL_error(L, "Couldn't create reflection information.");
        return 0;
    }

    D3D11_SHADER_DESC desc;
    ShaderHeader header = {0};
    Heart::hInputLayoutDesc* inLayout = NULL;
    reflect->GetDesc( &desc );
    if (progtype == proto::eShaderType_Vertex)
    {
        inLayout = (Heart::hInputLayoutDesc*)hAlloca(sizeof(Heart::hInputLayoutDesc)*desc.InputParameters);
        header.vertexLayout = 0;
        header.inputLayoutElements = ParseVertexInputFormat(desc, reflect, &shaderresource);
        if (header.inputLayoutElements == hErrorCode) {
            luaL_error(L, "Failed to parse vertex input format from shader");
            return 0;
        }
    }

    header.resHeader.resourceType = SHADER_MAGIC_NUM;
    header.version = SHADER_VERSION;
    //header.type = progtype;
    header.shaderBlobSize = (hUint32)result->GetBufferSize();
    if (header.shaderBlobSize != result->GetBufferSize()) {
        luaL_error(L, "Shader blob is too large (larger than 2GB?");
        return 0;
    }

    lua_getglobal(L, "buildpathresolve");
    lua_pushvalue(L, 4);
    lua_call(L, 1, 1);
    const hChar* outputpath=lua_tostring(L, -1);
    std::ofstream output;
    output.open(outputpath, std::ios_base::out|std::ios_base::binary);
    if (!output.is_open()) {
        luaL_error(L, "Unable to open output file %s", outputpath);
        return 0;
    }

    std::string streambuffer;
    google::protobuf::io::StringOutputStream filestream(&streambuffer);
    google::protobuf::io::CodedOutputStream outputstream(&filestream);

    //write the resource header
    Heart::proto::ResourceHeader resHeader;
    resHeader.set_type("gpu");
    resHeader.set_sourcefile(lua_tostring(L, 4));
    Heart::proto::ResourceSection* blobsection = resHeader.add_sections();
    blobsection->set_type(Heart::proto::eResourceSection_Temp);
    blobsection->set_sectionname("shader");
    blobsection->set_size(shaderresource.ByteSize());

    //output the source sections desc
    if (includesource) {
        Heart::proto::ResourceSection* sourcesection = resHeader.add_sections();
        sourcesection->set_type(Heart::proto::eResourceSection_Temp);
        sourcesection->set_sectionname("source");
        sourcesection->set_size(sourcedatalen);
        for (auto i=includeHandler.includedFiles_.begin(), n=includeHandler.includedFiles_.end(); i!=n; ++i) {
            if (i->second.datasize_ > 0) {
                Heart::proto::ResourceSection* includesection = resHeader.add_sections();
                includesection->set_type(Heart::proto::eResourceSection_Temp);
                includesection->set_sectionname(i->second.includestring_);
                includesection->set_size(i->second.datasize_);
            }
        }
    }

    Heart::serialiseToStreamWithSizeHeader(resHeader, &outputstream);
    shaderresource.SerializeToCodedStream(&outputstream);

    //output the source sections
    if (includesource) {
        outputstream.WriteRaw(sourcedata.get(), sourcedatalen);
        for (auto i=includeHandler.includedFiles_.begin(), n=includeHandler.includedFiles_.end(); i!=n; ++i) {
            if (i->second.datasize_ > 0) {
                outputstream.WriteRaw((char*)i->second.data_.get(), i->second.datasize_);
            }
        }
    }

    output.write(streambuffer.c_str(), streambuffer.length());
    output.close();
    lua_pop(L, 1);

    lua_newtable(L); // push empty table of resources this is dependent on (which is always none)

    lua_newtable(L); // push table of files files that where included by 
    hUint idx=1;
    for (FXIncludeHandler::IncludeMap::iterator i=includeHandler.includedFiles_.begin(), n=includeHandler.includedFiles_.end(); i!=n; ++i) {
        lua_pushstring(L, i->second.fullpath_.generic_string().c_str());
        lua_rawseti(L, -2, idx);
        ++idx;
    }

    result->Release();
    result=hNullptr;
    return 2;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int SB_API shaderPreprocess(lua_State* L) {
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 ParseVertexInputFormat(const D3D11_SHADER_DESC &desc, ID3D11ShaderReflection* reflect, Heart::proto::ShaderResource* shaderresource)
{
    hUint32 vertexInputLayoutFlags = 0;
    for ( hUint32 i = 0; i < desc.InputParameters; ++i )
    {
        Heart::proto::ShaderInputStream* stream=shaderresource->add_inputstreams();
        D3D11_SIGNATURE_PARAMETER_DESC inputDesc;
        reflect->GetInputParameterDesc( i, &inputDesc );

        stream->set_semantic(inputDesc.SemanticName);
        stream->set_semanticindex(inputDesc.SemanticIndex);
        switch(inputDesc.Mask)
        {
        case 0x01: stream->set_elementcount(1); break;
        case 0x03: stream->set_elementcount(2); break;
        case 0x07: stream->set_elementcount(3); break;
        case 0x0F: stream->set_elementcount(4); break;
        default: return hErrorCode;
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
        {"compile",shaderCompiler},
        {"preprocess",shaderPreprocess},
        {NULL, NULL}
    };
    luaL_newlib(L, gpuproglib);
    //lua_setglobal(L, "gpuprogram");
    return 1;
}
};
