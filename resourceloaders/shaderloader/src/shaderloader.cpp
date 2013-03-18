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

    FXIncludeHandler() {}
    ~FXIncludeHandler() {
        while (includePaths_.GetHead()) {
            FXIncludePath* inc=includePaths_.GetHead();
            includePaths_.Remove(inc);
            hDELETE(Heart::GetGlobalHeap(), inc); 
        }
    }

    void addDefaultPath(const hChar* path) {
        FXIncludePath* inc=hNEW(Heart::GetGlobalHeap(), FXIncludePath)();
        inc->path_=path;
        includePaths_.PushBack(inc);
    }

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
void HEART_API HeartGetBuilderVersion(hUint32* verMajor, hUint32* verMinor) {
    *verMajor = 1;
    *verMinor = 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
Heart::hResourceClassBase* HEART_API HeartBinLoader( Heart::hISerialiseStream* inFile, Heart::hIDataParameterSet*, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine)
{
    using namespace Heart;
    hShaderProgram* shaderProg=NULL;
    ShaderHeader header;
    hInputLayoutDesc* inLayout = NULL;
    void* shaderBlob = NULL;
    inFile->Read(&header, sizeof(ShaderHeader));
    hcAssert(header.version == SHADER_VERSION);
    if (header.inputLayoutElements) {
        inLayout = (hInputLayoutDesc*)hAlloca(sizeof(hInputLayoutDesc)*header.inputLayoutElements);
        inFile->Read(inLayout, sizeof(hInputLayoutDesc)*header.inputLayoutElements);
    }

    shaderBlob = hHeapMalloc(memalloc->tempHeap_, header.shaderBlobSize);
    inFile->Read(shaderBlob, header.shaderBlobSize);

    engine->GetRenderer()->createShader(
        memalloc->resourcePakHeap_,
        (hChar*)shaderBlob, header.shaderBlobSize,
        header.type, &shaderProg);

    hHeapFreeSafe(memalloc->tempHeap_, shaderBlob);

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
    const char* entry = params->GetBuildParameter("ENTRY","main");
    const char* profile = params->GetBuildParameter("PROFILE","vs_5_0");
    hShaderProfile profileType=hRenderer::getProfileFromString(profile);

    if (profileType >= eShaderProfile_vs4_0 && profileType <= eShaderProfile_vs5_0) {
        progtype = ShaderType_VERTEXPROG;
    } else if (profileType >= eShaderProfile_ps4_0 && profileType <= eShaderProfile_ps5_0) {
        progtype = ShaderType_FRAGMENTPROG;
    } else if (profileType >= eShaderProfile_gs4_0 && profileType <= eShaderProfile_gs5_0) {
        progtype = ShaderType_GEOMETRYPROG;
    } else if (profileType >= eShaderProfile_cs4_0 && profileType <= eShaderProfile_cs5_0) {
        progtype = ShaderType_COMPUTEPROG;
    } else if (profileType == eShaderProfile_hs5_0) {
        progtype = ShaderType_HULLPROG;    
    } else if (profileType == eShaderProfile_ds5_0) {
        progtype = ShaderType_DOMAINPROG;
    }

    FXIncludeHandler includeHandler;
    HRESULT hr;
    hUint32 compileFlags = 0;
#if _DEBUG
    compileFlags |= D3DCOMPILE_DEBUG;
#endif
    ID3DBlob* errors;
    ID3DBlob* result;
    hUint definecount=params->getBuildParameterCount("DEFINE");
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
    D3D_SHADER_MACRO* fullmacros=(D3D_SHADER_MACRO*)hAlloca(sizeof(D3D_SHADER_MACRO)*(definecount+1+hStaticArraySize(defaultmacros)));
    for (hUint i=0; i<definecount; ++i) {
        fullmacros[i].Name=params->getBuildParameter("DEFINE", i, "__NULL_DEF");
        fullmacros[i].Definition=params->getBuildParameterAttrib("DEFINE", i, "value", NULL);
    }
    hMemCpy(fullmacros+definecount, defaultmacros, sizeof(defaultmacros));

    hUint pathlen=hStrLen(params->GetInputFilePath())+1;//+1 for NULL
    hChar* path=(hChar*)hAlloca(pathlen);
    hChar* pathrootend=NULL;
    hStrCopy(path, pathlen, params->GetInputFilePath());
    if ((pathrootend=hStrRChr(path, '\\')) != NULL || (pathrootend=hStrRChr(path, '/')) != NULL) {
        *(pathrootend+1)=0;
    } else {
        *path=0;
    }

    includeHandler.fileAccess_ = fileCache;
    includeHandler.addDefaultPath(path);

    hChar* sourcedata = NULL;
    hUint32 sourcedatalen = inFile->Lenght();
    sourcedata = (hChar*)hHeapRealloc(memalloc->tempHeap_, sourcedata, sourcedatalen+1);
    inFile->Read(sourcedata, sourcedatalen);
    sourcedata[sourcedatalen] = 0;

    hr = D3DCompile( 
        sourcedata, 
        sourcedatalen, 
        params->GetInputFilePath(),
        fullmacros, 
        &includeHandler, //Includes
        entry,
        profile, 
        compileFlags, 
        0, 
        &result, 
        &errors);

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
    sp->DecRef();
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