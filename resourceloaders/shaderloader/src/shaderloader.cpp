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

hUint32 ParseVertexInputFormat(const D3D11_SHADER_DESC &desc, ID3D11ShaderReflection* reflect, Heart::hInputLayoutDesc* output, hUint32 maxOut);

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
        str->~FXIncludePath();
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
    Heart::hShaderType           type;
    hUint32                     vertexLayout;
    hUint32                     shaderBlobSize;
    hUint32                     inputLayoutElements;
};

#pragma pack(pop)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
Heart::hResourceClassBase* HEART_API HeartBinLoader( Heart::hISerialiseStream* inFile, Heart::hIDataParameterSet*, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine)
{
    using namespace Heart;
    hShaderProgram* shaderProg = hNEW(memalloc->resourcePakHeap_, hShaderProgram)();
    ShaderHeader header;
    hInputLayoutDesc* inLayout = NULL;
    void* tmpShaderBlob = NULL;

    inFile->Read(&header, sizeof(ShaderHeader));

    hcAssert(header.version == SHADER_VERSION);

    if (header.inputLayoutElements)
    {
        inLayout = (hInputLayoutDesc*)hAlloca(sizeof(hInputLayoutDesc)*header.inputLayoutElements);
        inFile->Read(inLayout, sizeof(hInputLayoutDesc)*header.inputLayoutElements);
    }

    tmpShaderBlob = hHeapMalloc(memalloc->tempHeap_, header.shaderBlobSize);
    inFile->Read(tmpShaderBlob, header.shaderBlobSize);

    engine->GetRenderer()->CompileShader(
        (hChar*)tmpShaderBlob, header.shaderBlobSize, 
        inLayout, header.inputLayoutElements, 
        header.type, shaderProg);
    shaderProg->SetShaderType(header.type);
    shaderProg->SetVertexLayout(header.vertexLayout);

    hHeapFreeSafe(memalloc->tempHeap_, tmpShaderBlob);

    return shaderProg;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
hBool HEART_API HeartDataCompiler( Heart::hIDataCacheFile* inFile, Heart::hIBuiltDataCache* fileCache, Heart::hIDataParameterSet* params, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine, Heart::hISerialiseStream* binoutput )
{
    using namespace Heart;

    Heart::hShaderType progtype;
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
    sourcedata = (hChar*)hHeapRealloc(memalloc->tempHeap_, sourcedata, sourcedatalen+1);
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
        hHeapFreeSafe(memalloc->tempHeap_, sourcedata);
        return hFalse;
    }

    hHeapFreeSafe(memalloc->tempHeap_, sourcedata);

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
    Heart::hInputLayoutDesc* inLayout = NULL;
    if (progtype == ShaderType_VERTEXPROG)
    {
        inLayout = (Heart::hInputLayoutDesc*)hAlloca(sizeof(Heart::hInputLayoutDesc)*desc.InputParameters);
        header.vertexLayout = 0;
        header.inputLayoutElements = ParseVertexInputFormat(desc, reflect, inLayout, desc.InputParameters);
    }

    header.resHeader.resourceType = SHADER_MAGIC_NUM;
    header.version = SHADER_VERSION;
    header.type = progtype;
    header.shaderBlobSize = result->GetBufferSize();

    binoutput->Write(&header, sizeof(header));
    if (header.inputLayoutElements)
    {
        binoutput->Write(inLayout, sizeof(Heart::hInputLayoutDesc)*header.inputLayoutElements);
    }
    binoutput->Write(result->GetBufferPointer(), result->GetBufferSize());

    return hTrue;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
hBool HEART_API HeartPackageLink( Heart::hResourceClassBase* resource, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine )
{
    return hTrue;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
void HEART_API HeartPackageUnlink( Heart::hResourceClassBase* resource, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine )
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
void HEART_API HeartPackageUnload( Heart::hResourceClassBase* resource, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine )
{
    using namespace Heart;

    hShaderProgram* sp = static_cast<hShaderProgram*>(resource);
    engine->GetRenderer()->DestroyShader(sp);
    hDELETE(memalloc->resourcePakHeap_, sp);
}

hUint32 ParseVertexInputFormat(const D3D11_SHADER_DESC &desc, ID3D11ShaderReflection* reflect, Heart::hInputLayoutDesc* output, hUint32 maxOut)
{
    hUint32 vertexInputLayoutFlags = 0;
    for ( hUint32 i = 0; i < desc.InputParameters && i < maxOut; ++i )
    {
        D3D11_SIGNATURE_PARAMETER_DESC inputDesc;
        Heart::hInputLayoutDesc* vid = output+i;
        reflect->GetInputParameterDesc( i, &inputDesc );
        const hChar* semantic = NULL;
        hChar inputStr[4] = {0};
        hUint32 streamIdx = 0;
        if (inputDesc.SemanticName[i] == 'i')
        {
            inputStr[0] = inputDesc.SemanticName[0];
            inputStr[1] = inputDesc.SemanticName[1];
            inputStr[2] = inputDesc.SemanticName[2];
            streamIdx = Heart::hAtoI(inputStr+1);
        }

        vid->instanceDataRepeat_ = 0;
        
        if ( Heart::hStrCmp( inputDesc.SemanticName, "POSITION" ) == 0 )
        {
            vid->semantic_ = Heart::eIS_POSITION;
        }
        else if ( Heart::hStrCmp( inputDesc.SemanticName, "NORMAL" ) == 0 )
        {
            vid->semantic_ = Heart::eIS_NORMAL;
        }
        else if ( Heart::hStrCmp( inputDesc.SemanticName, "TANGENT" ) == 0 )
        {
            vid->semantic_ = Heart::eIS_TANGENT;
        }
        else if ( Heart::hStrCmp( inputDesc.SemanticName, "BITANGENT" ) == 0 )
        {
            vid->semantic_ = Heart::eIS_BITANGENT;
        }
        else if ( Heart::hStrCmp( inputDesc.SemanticName, "COLOR" ) == 0 )
        {
            vid->semantic_ = Heart::eIS_COLOUR;
        }
        else if ( Heart::hStrCmp( inputDesc.SemanticName, "TEXCOORD" ) == 0 )
        {
            vid->semantic_ = Heart::eIS_TEXCOORD;
        }
        else if ( Heart::hStrCmp( inputDesc.SemanticName, "INSTANCE" ) == 0 )
        {
            vid->semantic_ = Heart::eIS_INSTANCE;
            //TODO: encode this in semantic?
            vid->instanceDataRepeat_ = 1;
        }
        else
        {
            hcAssertFailMsg( "Unknown input semantic %s for vertex program", inputDesc.SemanticName );
        }

        vid->semIndex_ = inputDesc.SemanticIndex;

        if (inputDesc.ComponentType == D3D10_REGISTER_COMPONENT_FLOAT32)
        {
            switch(inputDesc.Mask)
            {
            case 0x01: vid->typeFormat_ = Heart::eIF_FLOAT1; break;
            case 0x03: vid->typeFormat_ = Heart::eIF_FLOAT2; break;
            case 0x07: vid->typeFormat_ = Heart::eIF_FLOAT3; break;
            case 0x0F: vid->typeFormat_ = Heart::eIF_FLOAT4; break;
            default: hcAssertFailMsg("Invalid write mask %u", inputDesc.Mask);
            }
        }
        else if (inputDesc.ComponentType == D3D10_REGISTER_COMPONENT_UINT32)
        {
            //TODO: validate this
            switch(inputDesc.Mask)
            {
            //case 0x01: vid->typeFormat_ = Heart::eIF_FLOAT1; break;
            //case 0x03: vid->typeFormat_ = Heart::eIF_FLOAT2; break;
            //case 0x07: vid->typeFormat_ = Heart::eIF_FLOAT3; break;
            case 0x0F: vid->typeFormat_ = Heart::eIF_UBYTE4_UNORM; break;
            default: hcAssertFailMsg("Invalid write mask %u", inputDesc.Mask);
            }
        }
        else if (inputDesc.ComponentType == D3D10_REGISTER_COMPONENT_SINT32)
        {
            //TODO: validate this
            switch(inputDesc.Mask)
            {
                //case 0x01: vid->typeFormat_ = Heart::eIF_FLOAT1; break;
                //case 0x03: vid->typeFormat_ = Heart::eIF_FLOAT2; break;
                //case 0x07: vid->typeFormat_ = Heart::eIF_FLOAT3; break;
            case 0x0F: vid->typeFormat_ = Heart::eIF_UBYTE4_SNORM; break;
            default: hcAssertFailMsg("Invalid write mask %u", inputDesc.Mask);
            }
        }

        vid->inputStream_ = streamIdx;
    }

    return desc.InputParameters;
}