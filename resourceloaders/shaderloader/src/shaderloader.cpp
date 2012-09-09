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

#include "shaderloader.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define SHADER_MAGIC_NUM              hMAKE_FOURCC('h','G','P','U')
#define SHADER_STRING_MAX_LEN         (32)
#define SHADER_MAJOR_VERSION          (((hUint16)1))
#define SHADER_MINOR_VERSION          (((hUint16)0))
#define SHADER_VERSION                ((SHADER_MAJOR_VERSION << 16)|SHADER_MINOR_VERSION)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 ParseVertexInputFormat( D3D11_SHADER_DESC &desc, ID3D11ShaderReflection* reflect );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct FXIncludeHandler : public ID3DInclude 
{
    struct FXIncludePath : Heart::hLinkedListElement< FXIncludePath >
    {
        Heart::hString path_;
    };
    typedef Heart::hLinkedList< FXIncludePath > IncludePathArray;

    STDMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) 
    {
        using namespace Heart;

        includePaths_.SetAutoDelete(hFalse);

        FXIncludePath* i = NULL;
        do
        {
            hString str;
            if (i)
            {
                str = (i->path_);
                str += "/";
                i = i->GetNext();
            }
            else
            {
                i = includePaths_.GetHead();
            }
            str += pFileName;
            Heart::hIDataCacheFile* f = fileAccess_->OpenFile( str.c_str() );

            if ( !f )
            {
                continue;
            }

            FXIncludePath* ptr = (FXIncludePath*)hNEW_ARRAY(GetGlobalHeap(), hChar, f->Lenght()+1+sizeof(FXIncludePath));
            hPLACEMENT_NEW(ptr) FXIncludePath;

            hByte* buffer = (hByte*)(ptr+1);
            f->Seek(0, hIDataCacheFile::BEGIN);
            f->Read( buffer, f->Lenght() );
            buffer[f->Lenght()] = 0;

            *ppData = buffer;
            *pBytes = (UINT)f->Lenght();

            str.RemoveAfterLastInc('/');
            ptr->path_ = str;

            includePaths_.PushBack(ptr);

            fileAccess_->CloseFile(f);

            return S_OK;
        }
        while (i);

        return E_FAIL;
    }

    STDMETHOD(Close)(THIS_ LPCVOID pData)
    {
        using namespace Heart;

        FXIncludePath* str = (FXIncludePath*)(((hByte*)pData)-sizeof(FXIncludePath));
        hChar* buf = (hChar*)str;
        includePaths_.Remove(str);
        hDELETE_ARRAY(GetGlobalHeap(), buf);

        return S_OK;
    }

    Heart::hIBuiltDataCache* fileAccess_;
    IncludePathArray         includePaths_;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)

struct ShaderHeader
{
    Heart::hResourceBinHeader   resHeader;
    hUint32                     version;
    Heart::ShaderType           type;
    hUint32                     vertexLayout;
    hUint32                     shaderBlobSize;
};

#pragma pack(pop)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
Heart::hResourceClassBase* HEART_API HeartBinLoader( Heart::hISerialiseStream* inFile, Heart::hIDataParameterSet*, Heart::HeartEngine* engine)
{
    using namespace Heart;
    hShaderProgram* shaderProg = hNEW(GetGlobalHeap(), hShaderProgram)();
    ShaderHeader header;
    void* tmpShaderBlob = NULL;

    inFile->Read(&header, sizeof(ShaderHeader));

    hcAssert(header.version == SHADER_VERSION);

    tmpShaderBlob = hHeapMalloc(GetGlobalHeap(), header.shaderBlobSize);
    inFile->Read(tmpShaderBlob, header.shaderBlobSize);

    hdShaderProgram* impl = engine->GetRenderer()->CompileShader((hChar*)tmpShaderBlob, header.shaderBlobSize, header.vertexLayout, header.type);
    shaderProg->SetImpl(impl);
    shaderProg->SetShaderType(header.type);
    shaderProg->SetVertexLayout(header.vertexLayout);

    hHeapFreeSafe(GetGlobalHeap(), tmpShaderBlob);

    return shaderProg;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
hBool HEART_API HeartDataCompiler( Heart::hIDataCacheFile* inFile, Heart::hIBuiltDataCache* fileCache, Heart::hIDataParameterSet* params, Heart::HeartEngine* engine, Heart::hISerialiseStream* binoutput )
{
    using namespace Heart;

    Heart::ShaderType progtype;
    const hChar* typestr = params->GetBuildParameter("TYPE","VERTEX");
    if (hStrICmp(typestr, "VERTEX") == 0)
        progtype = ShaderType_VERTEXPROG;
    else 
        progtype = ShaderType_FRAGMENTPROG;

    FXIncludeHandler includeHandler;
    HRESULT hr;
    hUint32 compileFlags = 0;
#if _DEBUG
    compileFlags |= D3DCOMPILE_DEBUG;
#endif
    ID3DBlob* errors;
    ID3DBlob* result;
    D3D_SHADER_MACRO macros[] =
    {
        { "HEART_USING_HLSL", "1" },
        { "HEART_USING_HLSL", "1" },
        { "HEART_ENGINE", "1" },
        { progtype == Heart::ShaderType_VERTEXPROG ? "HEART_COMPILE_VERT_PROGRAM" : "HEART_COMPILE_FRAG_PROGRAM", "1" },
        { NULL, NULL }
    };

    includeHandler.fileAccess_ = fileCache;

    hChar* sourcedata = NULL;
    hUint32 sourcedatalen = inFile->Lenght();
    sourcedata = (hChar*)hHeapRealloc(GetGlobalHeap(), sourcedata, sourcedatalen+1);
    inFile->Read(sourcedata, sourcedatalen);
    sourcedata[sourcedatalen] = 0;

    const char* entry = params->GetBuildParameter("ENTRY","main");
    const char* profile = params->GetBuildParameter("PROFILE","vs_3_0");
    hr = D3DX11CompileFromMemory( 
        sourcedata, 
        sourcedatalen, 
        params->GetInputFilePath(),
        macros, 
        &includeHandler, //Includes
        entry,
        profile, 
        compileFlags, 
        0, 
        NULL, 
        &result, 
        &errors, 
        NULL );

    if (FAILED(hr) && errors)
    {
        hcPrintf("Shader Compile failed! Error Msg :: %s", errors->GetBufferPointer());
        errors->Release();
        errors = NULL;
        hHeapFreeSafe(GetGlobalHeap(), sourcedata);
        return hFalse;
    }

    hHeapFreeSafe(GetGlobalHeap(), sourcedata);

    ID3D11ShaderReflection* reflect;
    hr = D3DReflect( result->GetBufferPointer(), result->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflect );
    if ( FAILED( hr ) )
    {
        hcAssertFailMsg( "Couldn't create reflection information." );
        return hFalse;
    }

    D3D11_SHADER_DESC desc;

    reflect->GetDesc( &desc );

    ShaderHeader header = {0};
    if (progtype == ShaderType_VERTEXPROG)
    {
        header.vertexLayout = ParseVertexInputFormat(desc, reflect);
    }

    header.resHeader.resourceType = SHADER_MAGIC_NUM;
    header.version = SHADER_VERSION;
    header.type = progtype;
    header.shaderBlobSize = result->GetBufferSize();

    binoutput->Write(&header, sizeof(header));
    binoutput->Write(result->GetBufferPointer(), result->GetBufferSize());

    return hTrue;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
hBool HEART_API HeartPackageLink( Heart::hResourceClassBase* resource, Heart::HeartEngine* engine )
{
    return hTrue;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
void HEART_API HeartPackageUnlink( Heart::hResourceClassBase* resource, Heart::HeartEngine* engine )
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
void HEART_API HeartPackageUnload( Heart::hResourceClassBase* resource, Heart::HeartEngine* engine )
{

}

hUint32 ParseVertexInputFormat( D3D11_SHADER_DESC &desc, ID3D11ShaderReflection* reflect )
{
    hUint32 vertexInputLayoutFlags = 0;
    for ( hUint32 i = 0; i < desc.InputParameters; ++i )
    {
        D3D11_SIGNATURE_PARAMETER_DESC inputDesc;
        reflect->GetInputParameterDesc( i, &inputDesc );
        
        if ( Heart::hStrCmp( inputDesc.SemanticName, "POSITION" ) == 0 )
        {
            vertexInputLayoutFlags |= Heart::hrVF_XYZ;
        }
        else if ( Heart::hStrCmp( inputDesc.SemanticName, "NORMAL" ) == 0 )
        {
            vertexInputLayoutFlags |= Heart::hrVF_NORMAL;
        }
        else if ( Heart::hStrCmp( inputDesc.SemanticName, "TANGENT" ) == 0 )
        {
            vertexInputLayoutFlags |= Heart::hrVF_TANGENT;
        }
        else if ( Heart::hStrCmp( inputDesc.SemanticName, "BINORMAL" ) == 0 )
        {
            vertexInputLayoutFlags |= Heart::hrVF_BINORMAL;
        }
        else if ( Heart::hStrCmp( inputDesc.SemanticName, "COLOR" ) == 0 ||
                  Heart::hStrCmp( inputDesc.SemanticName, "COLOR0" ) == 0 )
        {
            vertexInputLayoutFlags |= Heart::hrVF_COLOR;
        }
        else if ( Heart::hStrCmp( inputDesc.SemanticName, "TEXCOORD" ) == 0 )
        {
            vertexInputLayoutFlags |= Heart::hrVF_1UV << inputDesc.SemanticIndex;
        }
        else
        {
            hcAssertFailMsg( "Unknown input semantic %s for vertex program", inputDesc.SemanticName );
        }
    }

    return vertexInputLayoutFlags;
}