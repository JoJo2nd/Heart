/********************************************************************

    filename:   material_utils.cpp  
    
    Copyright (c) 19:1:2013 James Moran
    
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

#include "material_utils.h"
#include "boost/filesystem.hpp"
#include "xml_helpers.h"
#include "boost/smart_ptr.hpp"
#include <assert.h>
#include <d3d11.h>

class HLSLInclude : public ID3DInclude
{
public:
    struct HLSLIncludePath
    {
        boost::filesystem::path pathroot;
        const void*             data;
    };

    STDMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) 
    {
        using namespace Heart;

        {
            boost::filesystem::path incpath=boost::filesystem::absolute(pFileName);
            std::ifstream hlslfile;
            hlslfile.open(incpath.generic_string());
            if (hlslfile.is_open()) {
                size_t len=boost::filesystem::file_size(incpath);
                char* data=new char[len];
                *ppData=data;
                *pBytes=len;
                memset(data, 0, len);
                hlslfile.read(data, len);
                HLSLIncludePath hlslsrc;
                hlslsrc.pathroot = incpath.parent_path();
                hlslsrc.data=(*ppData);
                includePaths_.push_back(hlslsrc);
                return S_OK;
            }
        }

        for (size_t i=0,n=includePaths_.size(); i<n; ++i) {
            boost::filesystem::path incpath=boost::filesystem::absolute(includePaths_[i].pathroot / pFileName);
            std::ifstream hlslfile;
            hlslfile.open(incpath.generic_string());
            if (hlslfile.is_open()) {
                size_t len=boost::filesystem::file_size(incpath);
                char* data=new char[len];
                *ppData=data;
                *pBytes=len;
                memset(data, 0, len);
                hlslfile.read(data, len);
                HLSLIncludePath hlslsrc;
                hlslsrc.pathroot = incpath.parent_path();
                hlslsrc.data=(*ppData);
                includePaths_.push_back(hlslsrc);
                return S_OK;
            }
        }

        return E_FAIL;
    }

    STDMETHOD(Close)(THIS_ LPCVOID pData)
    {
        for (size_t i=0,n=includePaths_.size(); i<n; ++i) {
            if (pData==includePaths_[i].data) {
                includePaths_.erase(includePaths_.begin()+i);
                delete pData;
                return S_OK;
            }
        }
        return E_FAIL;
    }

    std::vector< HLSLIncludePath > includePaths_;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#if 0
bool extractVertexInputLayoutFromShaderSource(
    const char* source, uint len, const char* profile, 
    const char* entry, const char* filepath, const char* packagepath,
    std::vector< Heart::hInputLayoutDesc >* outLayout, std::string* outErrors) {
    assert(source && profile && entry && filepath && outLayout && outErrors);
    //FXIncludeHandler includeHandler;
    HRESULT hr;
    hUint32 compileFlags = 0;
    ID3DBlob* errors;
    ID3DBlob* result;
    HLSLInclude includeHandler;
    D3D_SHADER_MACRO macros[] =
    {
        { "HEART_USING_HLSL", "1" },
        { "HEART_USING_HLSL", "1" },
        { "HEART_ENGINE", "1" },
        { "HEART_COMPILE_VERT_PROGRAM", "1" },
        { NULL, NULL }
    };

    HLSLInclude::HLSLIncludePath basepath;
    basepath.pathroot=boost::filesystem::path(filepath).parent_path();
    basepath.data=NULL;
    includeHandler.includePaths_.push_back(basepath);
    basepath.pathroot=boost::filesystem::path(packagepath);
    includeHandler.includePaths_.push_back(basepath);

    hr = D3DCompile( 
        source, 
        len, 
        filepath,
        macros, 
        &includeHandler, //Includes
        entry,
        profile, 
        compileFlags, 
        0, 
        &result, 
        &errors);

    if (outErrors && errors){
        *outErrors = (const char*)errors->GetBufferPointer();
        *outErrors += "\n";
        errors->Release();
        errors=NULL;
    }

    if (FAILED(hr)) {
        return false;
    }

    ID3D11ShaderReflection* reflect;
    hr = D3DReflect( result->GetBufferPointer(), result->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflect );
    if (FAILED(hr)) {
        if (outErrors) {
            *outErrors += "Failed to create reflection interface";
        }
        return hFalse;
    }

    D3D11_SHADER_DESC desc;
    reflect->GetDesc( &desc );
    bool parseOK=true;
    for ( hUint32 i = 0; i < desc.InputParameters; ++i )
    {
        D3D11_SIGNATURE_PARAMETER_DESC inputDesc;
        Heart::hInputLayoutDesc vid;
        reflect->GetInputParameterDesc( i, &inputDesc );
        const hChar* semantic = NULL;

        vid.instanceDataRepeat_ = 0;

        if ( Heart::hStrCmp( inputDesc.SemanticName, "POSITION" ) == 0 ) {
            vid.semantic_ = Heart::eIS_POSITION;
        } else if ( Heart::hStrCmp( inputDesc.SemanticName, "NORMAL" ) == 0 ) {
            vid.semantic_ = Heart::eIS_NORMAL;
        } else if ( Heart::hStrCmp( inputDesc.SemanticName, "TANGENT" ) == 0 ) {
            vid.semantic_ = Heart::eIS_TANGENT;
        } else if ( Heart::hStrCmp( inputDesc.SemanticName, "BITANGENT" ) == 0 ) {
            vid.semantic_ = Heart::eIS_BITANGENT;
        } else if ( Heart::hStrCmp( inputDesc.SemanticName, "COLOR" ) == 0 ) {
            vid.semantic_ = Heart::eIS_COLOUR;
        } else if ( Heart::hStrCmp( inputDesc.SemanticName, "TEXCOORD" ) == 0 ) {
            vid.semantic_ = Heart::eIS_TEXCOORD;
        } else if (  Heart::hStrCmp( inputDesc.SemanticName, "INSTANCE" ) ) {
            vid.semantic_ = Heart::eIS_INSTANCE;
        } else {
            *outErrors += "Unknown input semantic ";
            *outErrors += inputDesc.SemanticName;
            *outErrors += "\n";
        }

        vid.semIndex_ = inputDesc.SemanticIndex;

        if (inputDesc.ComponentType == D3D10_REGISTER_COMPONENT_FLOAT32) {
            switch(inputDesc.Mask)  {
            case 0x01: vid.typeFormat_ = Heart::eIF_FLOAT1; break;
            case 0x03: vid.typeFormat_ = Heart::eIF_FLOAT2; break;
            case 0x07: vid.typeFormat_ = Heart::eIF_FLOAT3; break;
            case 0x0F: vid.typeFormat_ = Heart::eIF_FLOAT4; break;
            default: 
                parseOK=false; 
                *outErrors += "Unknown component type for input";
                *outErrors += inputDesc.SemanticName;
                *outErrors += "\n";
            }
        } else if (inputDesc.ComponentType == D3D10_REGISTER_COMPONENT_UINT32) {
            //TODO: validate this
            switch(inputDesc.Mask) {
                //case 0x01: vid.typeFormat_ = Heart::eIF_FLOAT1; break;
                //case 0x03: vid.typeFormat_ = Heart::eIF_FLOAT2; break;
                //case 0x07: vid.typeFormat_ = Heart::eIF_FLOAT3; break;
            case 0x0F: vid.typeFormat_ = Heart::eIF_UBYTE4_UNORM; break;
            default:
                parseOK=false; 
                *outErrors += "Unknown component type for input";
                *outErrors += inputDesc.SemanticName;
                *outErrors += "\n";
            }
        } else if (inputDesc.ComponentType == D3D10_REGISTER_COMPONENT_SINT32) {
            //TODO: validate this
            switch(inputDesc.Mask) {
                //case 0x01: vid.typeFormat_ = Heart::eIF_FLOAT1; break;
                //case 0x03: vid.typeFormat_ = Heart::eIF_FLOAT2; break;
                //case 0x07: vid.typeFormat_ = Heart::eIF_FLOAT3; break;
            case 0x0F: vid.typeFormat_ = Heart::eIF_UBYTE4_SNORM; break;
            default:
                parseOK=false; 
                *outErrors += "Unknown component type for input";
                *outErrors += inputDesc.SemanticName;
                *outErrors += "\n";
            }
        }

        vid.inputStream_ = 0;
        outLayout->push_back(vid);
    }

    result->Release();
    reflect->Release();
    return parseOK;
}
#endif
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void extractVertexProgramsFromMaterial(const boost::filesystem::path& xmlpath, const char* matxml, uint len, std::vector< std::string >* vertexProgNames) {
    xml_doc matxmldoc;
    matxmldoc.copy_parse(matxml, len);
    xml_getter node(matxmldoc);
    while (node.to_node()) {
        if (_stricmp(node.name(),"vertex")==0) {
            vertexProgNames->push_back(node.get_value_string());
        }
        if (_stricmp(node.name(),"material")==0 && node.get_attribute("inherit")) {
            boost::filesystem::path thispath;
            thispath=xmlpath.parent_path();
            thispath /= boost::filesystem::path(node.get_attribute("inherit")->value());
            std::string inheritpath=thispath.generic_string();

            std::ifstream infile;
            size_t xmlfilelen=0;
            infile.open(inheritpath.c_str());
            if (infile.is_open()) {
                xmlfilelen=boost::filesystem::file_size(inheritpath);
                boost::shared_array<char> xmldata(new char[xmlfilelen+1]);
                memset(xmldata.get(), 0, xmlfilelen+1);
                infile.read(xmldata.get(), xmlfilelen);
                xmldata[xmlfilelen]=0;
                //TODO: this is too broad and includes too many vertex programs, but it should be safe
                extractVertexProgramsFromMaterial(inheritpath, xmldata.get(), xmlfilelen, vertexProgNames);
            }
        }
        if (node.first_child(NULL).to_node()) {
            node=node.first_child(NULL);
        } else if (node.next_sibling().to_node()) {
            node=node.next_sibling();
        } else {
            while(node.to_node()) {
                if (node.parent().first_sibling(NULL).to_node()) {
                    node=node.parent().first_sibling(NULL);
                    break;
                } else {
                    node=node.parent();
                }
            }
        }
    }
}
