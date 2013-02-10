/********************************************************************

    filename: 	materialloader.cpp	
    
    Copyright (c) 29:7:2012 James Moran
    
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

#include "materialloader.h"



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define MATERIAL_MAGIC_NUM              hMAKE_FOURCC('h','M','F','X')
#define MATERIAL_STRING_MAX_LEN         (32)
#define MATERIAL_MAJOR_VERSION          (((hUint16)1))
#define MATERIAL_MINOR_VERSION          (((hUint16)0))
#define MATERIAL_VERSION                ((MATERIAL_MAJOR_VERSION << 16)|MATERIAL_MINOR_VERSION)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)

struct MaterialHeader
{
    Heart::hResourceBinHeader   resHeader;
    hUint32                     version;
    hByte                       samplerCount;
    hUint16                     samplerRemapCount;
    hUint16                     parameterCount;
    hUint32                     parameterRemapCount;
    hByte                       groupCount;
    hUint32                     techniqueCount;
    hUint32                     passCount;
    hChar                       defaultGroupName[MATERIAL_STRING_MAX_LEN];
    hUint64                     samplerOffset;      //always after header
    hUint64                     parameterOffset;
    hUint64                     groupOffset;
};

struct SamplerDefinition
{
    hChar                       samplerName[MATERIAL_STRING_MAX_LEN];
    Heart::hResourceID          defaultTextureID;
    Heart::hSamplerStateDesc    samplerState;
    hUint8                      remapParamCount;
};

struct ParameterDefinition
{
    hChar                       parameterName[MATERIAL_STRING_MAX_LEN];
    Heart::hParameterType       type;
};

struct GroupDefinition
{
    hChar               groupName[MATERIAL_STRING_MAX_LEN];
    hUint16             techniques;
};

struct TechniqueDefinition
{
    hChar               technqiueName[MATERIAL_STRING_MAX_LEN];
    hUint8              transparent;
    hUint8              layer;
    hUint16             passes;
};

struct PassDefintion
{
    hUint16                             pass;
    Heart::hBlendStateDesc              blendState;
    Heart::hDepthStencilStateDesc       depthState;
    Heart::hRasterizerStateDesc         rasterizerState;
    Heart::hResourceID                  vertexProgramID;
    Heart::hResourceID                  fragmentProgramID;
    Heart::hResourceID                  geometryProgramID;
    Heart::hResourceID                  hullProgramID;
    Heart::hResourceID                  domainProgramID;
};

#pragma pack(pop)

class TechniqueData : public Heart::hLinkedListElement< TechniqueData >
{
public:
    TechniqueData(Heart::hMemoryHeapBase* heap)
        : heap_(heap)
        , passes_(heap)
    {
    }
    HEART_PRIVATE_COPY(TechniqueData);
public:
    typedef Heart::hVector< PassDefintion > PassArrayType;

    Heart::hMemoryHeapBase* heap_;
    TechniqueDefinition     techDef_;
    PassArrayType           passes_;
};

class GroupData : public Heart::hLinkedListElement< GroupData >
{
public:
    GroupData(Heart::hMemoryHeapBase* heap) 
        : heap_(heap)
        , techniques_(heap)
    {}
    HEART_PRIVATE_COPY(GroupData);
public:
    typedef Heart::hLinkedList< TechniqueData > TechniqueListType;

    TechniqueData* addTechnique(){
        TechniqueData* data=hNEW(heap_, TechniqueData)(heap_);
        techniques_.PushBack(data);
        return data;
    }
    TechniqueData* getTechnique(const hChar* name) {
        for (TechniqueData* i=techniques_.GetHead(); i; i=i->GetNext()) {
            if (Heart::hStrCmp(name, i->techDef_.technqiueName)==0) {
                return i;
            }
        }
        return NULL;
    }

    Heart::hMemoryHeapBase* heap_;
    GroupDefinition         groupDef_;
    TechniqueListType       techniques_;
};

class MaterialData
{
public:
    MaterialData(Heart::hMemoryHeapBase* heap)
        : heap_(heap)
        , samplers_(heap)
        , groups_(heap)
        , parameters_(heap)
    {}
    HEART_PRIVATE_COPY(MaterialData);
public:
    typedef Heart::hVector< SamplerDefinition >     SamplerArrayType;
    typedef Heart::hVector< ParameterDefinition >   ParamArrayType;
    typedef Heart::hLinkedList< GroupData >         GroupListType;

    GroupData* addGroup() {
        GroupData* data=hNEW(heap_, GroupData)(heap_);
        groups_.PushBack(data);
        return data;
    }
    GroupData* getGroup(const hChar* name) {
        for (GroupData* i=groups_.GetHead(); i; i=i->GetNext()) {
            if (Heart::hStrCmp(name, i->groupDef_.groupName)==0) {
                return i;
            }
        }
        return NULL;
    }
    SamplerDefinition* getSamplerByName(const hChar* name) {
        for (hUint i=0,n=samplers_.GetSize(); i<n; ++i) {
            if (Heart::hStrCmp(name, samplers_[i].samplerName)==0) {
                return &samplers_[i];
            }
        }
        return NULL;
    }
    ParameterDefinition* getParameterByName(const hChar* name) {
        for (hUint i=0,n=parameters_.GetSize(); i<n; ++i) {
            if (Heart::hStrCmp(name, parameters_[i].parameterName)==0) {
                return &parameters_[i];
            }
        }
        return NULL;
    }

    Heart::hMemoryHeapBase* heap_;
    MaterialHeader          header_;
    SamplerArrayType        samplers_;
    ParamArrayType          parameters_;
    GroupListType           groups_;
};


//////////////////////////////////////////////////////////////////////////
// Enum Tables ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Heart::hXMLEnumReamp g_parameterTypes[] =
{
    {"float",   Heart::ePTFloat1},
    {"float2",  Heart::ePTFloat2},
    {"float3",  Heart::ePTFloat3},
    {"float4",  Heart::ePTFloat4},
    {"matrix3", Heart::ePTFloat3x3},
    {"matrix4", Heart::ePTFloat4x4},
    {"none",    Heart::ePTNone},
};


Heart::hXMLEnumReamp g_samplerStates[] =
{
    { "repeat",                  Heart::SSV_WRAP },
    { "wrap",                    Heart::SSV_WRAP },

    { "clamp",                   Heart::SSV_CLAMP },
    { "clamptoedge",             Heart::SSV_CLAMP },
    { "mirror",                  Heart::SSV_MIRROR },
    { "border",                  Heart::SSV_BORDER },
    { " ",             Heart::SSV_ANISOTROPIC },
    { "point",                   Heart::SSV_POINT },
    { "linear",                  Heart::SSV_LINEAR },
    { "linearmipmaplinear",      Heart::SSV_LINEAR },
    { NULL,                      0}
};

Heart::hXMLEnumReamp g_trueFalseEnum[] = 
{
    { "true", Heart::RSV_ENABLE },
    { "false", Heart::RSV_DISABLE },
    {NULL, 0}
};

Heart::hXMLEnumReamp g_blendFuncEnum[] = 
{
    { "funcadd" , Heart::RSV_BLEND_FUNC_ADD },
    { "funcsubtract" , Heart::RSV_BLEND_FUNC_SUB },
    { "add" , Heart::RSV_BLEND_FUNC_ADD },
    { "subtract" , Heart::RSV_BLEND_FUNC_SUB },
    { "min" , Heart::RSV_BLEND_FUNC_MIN },
    { "max" , Heart::RSV_BLEND_FUNC_MAX },
    {NULL, 0}
};
Heart::hXMLEnumReamp g_blendOpEnum[] =
{
    { "zero" , Heart::RSV_BLEND_OP_ZERO },
    { "one" , Heart::RSV_BLEND_OP_ONE },
    { "destcolor" , Heart::RSV_BLEND_OP_DEST_COLOUR },
    { "invdestcolor" , Heart::RSV_BLEND_OP_INVDEST_COLOUR },
    { "srcalpha" , Heart::RSV_BLEND_OP_SRC_ALPHA },
    { "invsrcalpha" , Heart::RSV_BLEND_OP_INVSRC_ALPHA },
    { "dstalpha" , Heart::RSV_BLEND_OP_DEST_ALPHA },
    { "invdestalpha" , Heart::RSV_BLEND_OP_INVDEST_ALPHA },
    { "srccolor" , Heart::RSV_BLEND_OP_SRC_COLOUR },
    { "invsrccolor" , Heart::RSV_BLEND_OP_INVSRC_COLOUR },
    {NULL, 0}
};
Heart::hXMLEnumReamp g_depthEnum[] = 
{
    { "never" , Heart::RSV_Z_CMP_NEVER },
    { "less" , Heart::RSV_Z_CMP_LESS },
    { "lequal" , Heart::RSV_Z_CMP_LESSEQUAL },
    { "lessequal" , Heart::RSV_Z_CMP_LESSEQUAL },
    { "equal" , Heart::RSV_Z_CMP_EQUAL },
    { "greater" , Heart::RSV_Z_CMP_GREATER },
    { "notequal" , Heart::RSV_Z_CMP_NOT_EQUAL },
    { "gequal" , Heart::RSV_Z_CMP_GREATER_EQUAL },
    { "greaterequal" , Heart::RSV_Z_CMP_GREATER_EQUAL },
    { "always" , Heart::RSV_Z_CMP_ALWAYS },
    {NULL, 0}
};
Heart::hXMLEnumReamp g_fillModeEnum[] = 
{
    { "solid" , Heart::RSV_FILL_MODE_SOLID },
    { "wireframe" , Heart::RSV_FILL_MODE_WIREFRAME },
    {NULL, 0}
};
Heart::hXMLEnumReamp g_cullModeEnum[] =
{
    { "none" , Heart::RSV_CULL_MODE_NONE },
    { "cw" , Heart::RSV_CULL_MODE_CW },
    { "ccw" , Heart::RSV_CULL_MODE_CCW },
    {NULL, 0}
};
Heart::hXMLEnumReamp g_stencilFuncEnum[] =
{
    { "never" , Heart::RSV_SF_CMP_NEVER },
    { "less" , Heart::RSV_SF_CMP_LESS },
    { "lequal" , Heart::RSV_SF_CMP_LESSEQUAL },
    { "lessequal" , Heart::RSV_SF_CMP_LESSEQUAL },
    { "equal" , Heart::RSV_SF_CMP_EQUAL },
    { "greater" , Heart::RSV_SF_CMP_GREATER },
    { "notequal" , Heart::RSV_SF_CMP_NOT_EQUAL },
    { "gequal" , Heart::RSV_SF_CMP_GREATER_EQUAL },
    { "greaterequal" , Heart::RSV_SF_CMP_GREATER_EQUAL },
    { "always" , Heart::RSV_SF_CMP_ALWAYS },
    {NULL, 0}
};
Heart::hXMLEnumReamp g_stencilOpEnum[] = 
{
    { "keep" ,      Heart::RSV_SO_KEEP, },
    { "zero" ,      Heart::RSV_SO_ZERO, },
    { "replace" ,   Heart::RSV_SO_REPLACE, },
    { "incr" ,      Heart::RSV_SO_INCR, },
    { "decr" ,      Heart::RSV_SO_DECR, },
    { "invert" ,    Heart::RSV_SO_INVERT, },
    { "incrsat" ,   Heart::RSV_SO_INCRSAT },
    { "decrsat" ,   Heart::RSV_SO_DECRSAT },
    {NULL, 0}
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void readMaterialXMLToMaterialData(const Heart::hXMLDocument& xmldoc, const hChar* xmlpath, 
Heart::hIBuiltDataCache* fileCache, Heart::hMemoryHeapBase* heap, MaterialData* mat);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
Heart::hResourceClassBase* HEART_API HeartBinLoader( Heart::hISerialiseStream* inFile, Heart::hIDataParameterSet*, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine )
{
    using namespace Heart;
    hRenderer* renderer = engine->GetRenderer();
    hMaterial* material = hNEW(memalloc->resourcePakHeap_, hMaterial)(memalloc->resourcePakHeap_, renderer);

    MaterialHeader header;
    inFile->Read(&header, sizeof(header));

    //TODO: handle this...
    hcAssert(header.version == MATERIAL_VERSION);

    //Read samplers
    for (hUint32 i = 0, imax = header.samplerCount; i < imax; ++i)
    {
        SamplerDefinition samplerDef;
        hSamplerParameter sampler;
        inFile->Read(&samplerDef, sizeof(samplerDef));
        
        sampler.defaultTextureID_ = samplerDef.defaultTextureID;
        hStrCopy(sampler.name_, sampler.name_.GetMaxSize(), samplerDef.samplerName);
        sampler.samplerState_ = renderer->CreateSamplerState(samplerDef.samplerState);

        material->AddSamplerParameter(sampler);
    }
/*
    material->SetParameterInputOutputReserves(header.parameterCount, header.parameterRemapCount);
*/
    //Read parameters
    for (hUint32 i = 0, imax = header.parameterCount; i < imax; ++i)
    {
        ParameterDefinition paramDef;
        //hMaterialParameterID id;
        inFile->Read(&paramDef, sizeof(paramDef));
    }

    //Add Groups, Techniques & Passes
    for (hUint32 groupidx = 0, groupCount = header.groupCount; groupidx < groupCount; ++groupidx)
    {
        GroupDefinition groupDef;
        hMaterialGroup* group = NULL;
        inFile->Read(&groupDef, sizeof(groupDef));

        group = material->AddGroup(groupDef.groupName);

        group->techniques_.Reserve(groupDef.techniques);
        group->techniques_.Resize(groupDef.techniques);
        for (hUint32 techniqueIdx = 0, techniqueCount = groupDef.techniques; techniqueIdx < techniqueCount; ++techniqueIdx)
        {
            TechniqueDefinition techDef;
            inFile->Read(&techDef, sizeof(techDef));
            hMaterialTechnique* tech = &group->techniques_[techniqueIdx];

            tech->SetName(techDef.technqiueName);
            tech->SetPasses(techDef.passes);
            tech->SetLayer(techDef.layer);
            tech->SetSort(techDef.transparent > 0);

            for (hUint32 passIdx = 0, passCount = techDef.passes; passIdx < passCount; ++passIdx)
            {
                PassDefintion passDef;
                hMaterialTechniquePass pass;

                inFile->Read(&passDef, sizeof(passDef));
                
                pass.SetBlendState(renderer->CreateBlendState(passDef.blendState));
                pass.SetDepthStencilState(renderer->CreateDepthStencilState(passDef.depthState));
                pass.SetRasterizerState(renderer->CreateRasterizerState(passDef.rasterizerState));
                pass.SetVertexShaderResID(passDef.vertexProgramID);
                pass.SetFragmentShaderResID(passDef.fragmentProgramID);

                tech->AppendPass(pass);
            }
        }
    }

    material->SetActiveGroup(header.defaultGroupName);

    return material;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
hBool HEART_API HeartDataCompiler( Heart::hIDataCacheFile* inFile, Heart::hIBuiltDataCache* fileCache, Heart::hIDataParameterSet* params, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine, Heart::hISerialiseStream* binoutput )
{
    using namespace Heart;
    MaterialHeader matHeader = {0};
    MaterialData matData(memalloc->tempHeap_);

    binoutput->Seek(0, hISerialiseStream::eBegin);

    hXMLDocument xmldoc;
    hChar* xmlmem = (hChar*)hHeapMalloc(memalloc->tempHeap_, inFile->Lenght()+1);
    inFile->Read(xmlmem, inFile->Lenght());
    xmlmem[inFile->Lenght()] = 0;

    if (xmldoc.ParseSafe< rapidxml::parse_default >(xmlmem, memalloc->tempHeap_) == hFalse)
        return NULL;

    readMaterialXMLToMaterialData(xmldoc, params->GetInputFilePath(), fileCache, memalloc->tempHeap_, &matData);

    matData.header_.samplerCount=matData.samplers_.GetSize();
    matData.header_.samplerOffset=0;
    matData.header_.parameterCount=matData.parameters_.GetSize();
    matData.header_.parameterOffset=0;
    matData.header_.groupCount=matData.groups_.GetSize();
    matData.header_.groupOffset=0;
    binoutput->Write(&matData.header_, sizeof(matData.header_));

    for (hUint samp=0,sampn=matData.header_.samplerCount; samp<sampn; ++samp) {
        binoutput->Write(&matData.samplers_[samp], sizeof(SamplerDefinition));
    }

    for (GroupData* grpData=matData.groups_.GetHead(); grpData; grpData=grpData->GetNext()) {
        grpData->groupDef_.techniques=grpData->techniques_.GetSize();
        binoutput->Write(&grpData->groupDef_, sizeof(grpData->groupDef_));
        for (TechniqueData* techData=grpData->techniques_.GetHead(); techData; techData=techData->GetNext()) {
            techData->techDef_.passes=techData->passes_.GetSize();
            binoutput->Write(&techData->techDef_, sizeof(techData->techDef_));
            for (hUint passidx=0,passidxn=techData->passes_.GetSize(); passidx<passidxn; ++passidx) {
                binoutput->Write(&techData->passes_[passidx], sizeof(PassDefintion));
            }
        }
    }
    
#if 0
    matHeader.resHeader.resourceType = hMAKE_FOURCC('X','X','X','X');//Write a invalid four cc, we'll write a correct one later
    matHeader.version = MATERIAL_VERSION;
    binoutput->Write(&matHeader, sizeof(matHeader));

    matHeader.resHeader.resourceType = MATERIAL_MAGIC_NUM;

    hStrCopy( matHeader.defaultGroupName, MATERIAL_STRING_MAX_LEN, hXMLGetter(&xmldoc).FirstChild("material").FirstChild("defaultgroup").GetValueString("lowdetail"));

    matHeader.samplerOffset = binoutput->Tell();

    hXMLGetter xSampler = hXMLGetter(&xmldoc).FirstChild("material").FirstChild("sampler");
    for (; xSampler.ToNode(); xSampler = xSampler.NextSibling())
    {
        SamplerDefinition sampDef = {0};
        hUint64 remapos = binoutput->Tell() + hOffsetOf(SamplerDefinition, remapParamCount);

        hStrCopy(sampDef.samplerName, MATERIAL_STRING_MAX_LEN, xSampler.GetAttributeString("name","none"));

        sampDef.defaultTextureID = hResourceManager::BuildResourceID(xSampler.FirstChild("texture").GetValueString());
        sampDef.samplerState.addressU_ = xSampler.FirstChild("addressu").GetValueEnum(g_samplerStates, SSV_CLAMP);
        sampDef.samplerState.addressV_ = xSampler.FirstChild("addressv").GetValueEnum(g_samplerStates, SSV_CLAMP);
        sampDef.samplerState.addressW_ = xSampler.FirstChild("addressw").GetValueEnum(g_samplerStates, SSV_CLAMP);
        sampDef.samplerState.borderColour_ = xSampler.FirstChild("bordercolour").GetValueColour(hColour(0.f, 0.f, 0.f, 1.f));
        sampDef.samplerState.filter_   = xSampler.FirstChild("filter").GetValueEnum(g_samplerStates, SSV_POINT);
        sampDef.samplerState.maxAnisotropy_ = xSampler.FirstChild("maxanisotropy").GetValueInt(1);
        sampDef.samplerState.minLOD_ = xSampler.FirstChild("minlod").GetValueFloat();
        sampDef.samplerState.maxLOD_ = xSampler.FirstChild("maxlod").GetValueFloat(FLT_MAX);
        sampDef.samplerState.mipLODBias_ = xSampler.FirstChild("miplodbias").GetValueFloat();

        ++matHeader.samplerCount;
        binoutput->Write(&sampDef, sizeof(sampDef));
    }

    matHeader.parameterOffset = binoutput->Tell();

    hXMLGetter xParameter = hXMLGetter(&xmldoc).FirstChild("material").FirstChild("parameter");
    for (; xParameter.ToNode(); xParameter = xParameter.NextSibling())
    {
        ParameterDefinition paramDef = {0};
        hUint64 remapos = binoutput->Tell()+hOffsetOf(ParameterDefinition, remapParamCount);
        
        hStrCopy(paramDef.parameterName, MATERIAL_STRING_MAX_LEN, xParameter.GetAttributeString("name","none"));
        paramDef.type = xParameter.GetAttributeEnum("type", g_parameterTypes, ePTNone );

        ++matHeader.parameterCount;
        binoutput->Write(&paramDef, sizeof(paramDef));
    }

    matHeader.groupOffset = binoutput->Tell();

    hXMLGetter xGroup = hXMLGetter(&xmldoc).FirstChild("material").FirstChild("group");
    for (; xGroup.ToNode(); xGroup = xGroup.NextSibling())
    {
        GroupDefinition groupDef = {0};
        hUint64 techos = binoutput->Tell() + hOffsetOf(GroupDefinition, techniques);

        hStrCopy(groupDef.groupName, MATERIAL_STRING_MAX_LEN, xGroup.GetAttributeString("name","none"));

        ++matHeader.groupCount;
        binoutput->Write(&groupDef, sizeof(groupDef));

        hXMLGetter xTech = xGroup.FirstChild("technique");
        for (; xTech.ToNode(); xTech = xTech.NextSibling())
        {
            TechniqueDefinition techDef = {0};
            hUint64 passos = binoutput->Tell() + hOffsetOf(TechniqueDefinition, passes);
            hStrCopy(techDef.technqiueName, MATERIAL_STRING_MAX_LEN, xTech.GetAttributeString("name","none"));
            techDef.transparent = hStrICmp(xTech.FirstChild("sort").GetValueString("false"), "true") == 0;
            techDef.layer = (hByte)(xTech.FirstChild("layer").GetValueInt()&0xFF);

            ++groupDef.techniques;
            ++matHeader.techniqueCount;
            binoutput->Write(&techDef, sizeof(techDef));

            hXMLGetter xPass = xTech.FirstChild("pass");
            for (; xPass.ToNode(); xPass = xPass.NextSibling())
            {
                PassDefintion passDef = {0};

                passDef.blendState.blendEnable_ = xPass.FirstChild("blendenable").GetValueEnum(g_trueFalseEnum, RSV_DISABLE);
                passDef.blendState.blendOp_ = xPass.FirstChild("blendop").GetValueEnum(g_blendFuncEnum, RSV_BLEND_FUNC_ADD);
                passDef.blendState.blendOpAlpha_ = xPass.FirstChild("blendopalpha").GetValueEnum(g_blendFuncEnum, RSV_BLEND_FUNC_ADD);
                passDef.blendState.destBlend_ = xPass.FirstChild("destblend").GetValueEnum(g_blendOpEnum, RSV_BLEND_OP_ONE);
                passDef.blendState.destBlendAlpha_ = xPass.FirstChild("destblendalpha").GetValueEnum(g_blendOpEnum, RSV_BLEND_OP_ONE);
                passDef.blendState.srcBlend_= xPass.FirstChild("srcblend").GetValueEnum(g_blendOpEnum, RSV_BLEND_OP_ONE);
                passDef.blendState.srcBlendAlpha_= xPass.FirstChild("srcblendalpha").GetValueEnum(g_blendOpEnum, RSV_BLEND_OP_ONE);
                passDef.blendState.renderTargetWriteMask_ = xPass.FirstChild("rendertargetwritemask").GetValueHex(0xFF);

                passDef.depthState.depthEnable_ = xPass.FirstChild("depthtest").GetValueEnum(g_trueFalseEnum, RSV_DISABLE);
                passDef.depthState.depthFunc_ = xPass.FirstChild("depthfunc").GetValueEnum(g_depthEnum, RSV_Z_CMP_LESS);
                passDef.depthState.depthWriteMask_ = xPass.FirstChild("depthwrite").GetValueEnum(g_trueFalseEnum, RSV_DISABLE);
                passDef.depthState.stencilEnable_ = xPass.FirstChild("stencilenable").GetValueEnum(g_trueFalseEnum, RSV_DISABLE);
                passDef.depthState.stencilFunc_ = xPass.FirstChild("stencilfunc").GetValueEnum(g_stencilFuncEnum, RSV_SF_CMP_NEVER);
                passDef.depthState.stencilDepthFailOp_ = xPass.FirstChild("stencildepthfailop").GetValueEnum(g_stencilOpEnum, RSV_SO_KEEP);
                passDef.depthState.stencilFailOp_ = xPass.FirstChild("stencilfail").GetValueEnum(g_stencilOpEnum, RSV_SO_KEEP);
                passDef.depthState.stencilPassOp_ = xPass.FirstChild("stencilpass").GetValueEnum(g_stencilOpEnum, RSV_SO_KEEP);
                passDef.depthState.stencilReadMask_ = xPass.FirstChild("stencilreadmask").GetValueHex(0xFFFFFFFF);
                passDef.depthState.stencilWriteMask_ = xPass.FirstChild("stencilwritemask").GetValueHex(0xFFFFFFFF);
                passDef.depthState.stencilRef_ = xPass.FirstChild("stencilref").GetValueHex(0x00000000);

                passDef.rasterizerState.cullMode_ = xPass.FirstChild("cullmode").GetValueEnum(g_cullModeEnum, RSV_CULL_MODE_CCW);
                passDef.rasterizerState.depthBias_ = 0;//xPass.FirstChild("depthbias").GetValueFloat();
                passDef.rasterizerState.depthBiasClamp_ = xPass.FirstChild("depthbiasclamp").GetValueFloat();
                passDef.rasterizerState.depthClipEnable_ = xPass.FirstChild("depthclipenable").GetValueEnum(g_trueFalseEnum, RSV_DISABLE);
                passDef.rasterizerState.fillMode_ = xPass.FirstChild("fillmode").GetValueEnum(g_fillModeEnum, RSV_FILL_MODE_SOLID);
                passDef.rasterizerState.slopeScaledDepthBias_ = xPass.FirstChild("slopescaleddepthbias").GetValueFloat();
                passDef.rasterizerState.scissorEnable_ = xPass.FirstChild("scissortest").GetValueEnum(g_trueFalseEnum, RSV_DISABLE);
                passDef.rasterizerState.frontCounterClockwise_ = RSV_ENABLE;

                passDef.vertexProgramID   = hResourceManager::BuildResourceID(xPass.FirstChild("vertex").GetValueString());
                passDef.fragmentProgramID = hResourceManager::BuildResourceID(xPass.FirstChild("fragment").GetValueString());

                ++techDef.passes;
                ++matHeader.passCount;
                binoutput->Write(&passDef, sizeof(passDef));
            }

            binoutput->Seek(passos, hISerialiseStream::eBegin);
            binoutput->Write(&techDef.passes, sizeof(techDef.passes));
            binoutput->Seek(0, hISerialiseStream::eEnd);
        }

        binoutput->Seek(techos, hISerialiseStream::eBegin);
        binoutput->Write(&groupDef.techniques, sizeof(groupDef.techniques));
        binoutput->Seek(0, hISerialiseStream::eEnd);
    }

    // Rewrite the header
    binoutput->Seek(0, hISerialiseStream::eBegin);
    binoutput->Write(&matHeader, sizeof(matHeader));
#endif

    return hTrue;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
hBool HEART_API HeartPackageLink( Heart::hResourceClassBase* resource, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine )
{
    using namespace Heart;
    hMaterial* mat = static_cast< hMaterial* >(resource);
    return mat->Link(engine->GetResourceManager(), engine->GetRenderer(), engine->GetRenderer()->GetMaterialManager());
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

    hMaterial* mat = static_cast<hMaterial*>(resource);
    hDELETE(memalloc->resourcePakHeap_, mat);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void readMaterialXMLToMaterialData(const Heart::hXMLDocument& xmldoc, const hChar* xmlpath, 
Heart::hIBuiltDataCache* fileCache, Heart::hMemoryHeapBase* heap, MaterialData* mat) {
    using namespace Heart;

    mat->header_.resHeader.resourceType = hMAKE_FOURCC('X','X','X','X');//Write a invalid four cc, we'll write a correct one later
    mat->header_.version = MATERIAL_VERSION;
    mat->header_.resHeader.resourceType = MATERIAL_MAGIC_NUM;
    hStrCopy(mat->header_.defaultGroupName, MATERIAL_STRING_MAX_LEN, hXMLGetter(&xmldoc).FirstChild("material").FirstChild("defaultgroup").GetValueString("lowdetail"));

    if (hXMLGetter(&xmldoc).FirstChild("material").GetAttributeString("inherit")) {
        //Load the base first
        hXMLDocument xmldocbase;
        const hChar* basepathrel=hXMLGetter(&xmldoc).FirstChild("material").GetAttributeString("inherit");
        hUint pathlen=hStrLen(basepathrel)+hStrLen(xmlpath)+1;//+1 for NULL
        hChar* fullbasepath=(hChar*)hAlloca(pathlen);
        hChar* pathSep=NULL;
        hStrCopy(fullbasepath, pathlen, xmlpath);
        pathSep=hStrRChr(fullbasepath, '/');
        if (pathSep) {
            *(pathSep+1)=0;
            hStrCat(fullbasepath, pathlen, basepathrel);
        } else {
            hStrCopy(fullbasepath, pathlen, basepathrel);
        }
        hIDataCacheFile* file=fileCache->OpenFile(fullbasepath);
        if (file) {
            hChar* ptr=(hChar*)hHeapMalloc(heap, file->Lenght()+1);
            hZeroMem(ptr, file->Lenght()+1);
            file->Read(ptr, file->Lenght());
            xmldocbase.ParseSafe< rapidxml::parse_default >(ptr, heap);
            fileCache->CloseFile(file);
            readMaterialXMLToMaterialData(xmldocbase, fullbasepath, fileCache, heap, mat);
        }
    }

    hXMLGetter xSampler = hXMLGetter(&xmldoc).FirstChild("material").FirstChild("sampler");
    for (; xSampler.ToNode(); xSampler = xSampler.NextSibling())
    {
        SamplerDefinition newSampDef = {0};
        SamplerDefinition* orgSampDef=mat->getSamplerByName(xSampler.GetAttributeString("name","none"));
        SamplerDefinition* sampDef= orgSampDef ? orgSampDef : &newSampDef;

        if (xSampler.GetAttribute("name") || !orgSampDef) {
            hStrCopy(sampDef->samplerName, MATERIAL_STRING_MAX_LEN, xSampler.GetAttributeString("name","none"));
        }
        if (xSampler.FirstChild("texture").ToNode() || !orgSampDef) {
            sampDef->defaultTextureID = hResourceManager::BuildResourceID(xSampler.FirstChild("texture").GetValueString());
        }
        if (xSampler.FirstChild("addressu").ToNode() || !orgSampDef) {
            sampDef->samplerState.addressU_ = xSampler.FirstChild("addressu").GetValueEnum(g_samplerStates, SSV_CLAMP);
        }
        if (xSampler.FirstChild("addressv").ToNode() || !orgSampDef) {
            sampDef->samplerState.addressV_ = xSampler.FirstChild("addressv").GetValueEnum(g_samplerStates, SSV_CLAMP);
        }
        if (xSampler.FirstChild("addressw").ToNode() || !orgSampDef) {
            sampDef->samplerState.addressW_ = xSampler.FirstChild("addressw").GetValueEnum(g_samplerStates, SSV_CLAMP);
        }
        if (xSampler.FirstChild("bordercolour").ToNode() || !orgSampDef) {
            sampDef->samplerState.borderColour_ = xSampler.FirstChild("bordercolour").GetValueColour(hColour(0.f, 0.f, 0.f, 1.f));
        }
        if (xSampler.FirstChild("filter").ToNode() || !orgSampDef) {
            sampDef->samplerState.filter_   = xSampler.FirstChild("filter").GetValueEnum(g_samplerStates, SSV_POINT);
        }
        if (xSampler.FirstChild("maxanisotropy").ToNode() || !orgSampDef) {
            sampDef->samplerState.maxAnisotropy_ = xSampler.FirstChild("maxanisotropy").GetValueInt(1);
        }
        if (xSampler.FirstChild("minlod").ToNode() || !orgSampDef) {
            sampDef->samplerState.minLOD_ = xSampler.FirstChild("minlod").GetValueFloat();
        }
        if (xSampler.FirstChild("maxlod").ToNode() || !orgSampDef) {
            sampDef->samplerState.maxLOD_ = xSampler.FirstChild("maxlod").GetValueFloat(FLT_MAX);
        }
        if (xSampler.FirstChild("miplodbias").ToNode() || !orgSampDef) {
            sampDef->samplerState.mipLODBias_ = xSampler.FirstChild("miplodbias").GetValueFloat();
        }

        if (!orgSampDef) {
            mat->samplers_.PushBack(*sampDef);
        }
    }

    hXMLGetter xParameter = hXMLGetter(&xmldoc).FirstChild("material").FirstChild("parameter");
    for (; xParameter.ToNode(); xParameter = xParameter.NextSibling())
    {
        ParameterDefinition newParamDef = {0};
        ParameterDefinition* orgParamDef=mat->getParameterByName(xParameter.GetAttributeString("name","none"));
        ParameterDefinition* paramDef=orgParamDef ? orgParamDef : &newParamDef;

        if (xParameter.GetAttribute("name") || !orgParamDef) {
            hStrCopy(paramDef->parameterName, MATERIAL_STRING_MAX_LEN, xParameter.GetAttributeString("name","none"));
        }
        if (xParameter.GetAttribute("type") || !orgParamDef) {
            paramDef->type = xParameter.GetAttributeEnum("type", g_parameterTypes, ePTNone );
        }
    }

    hXMLGetter xGroup = hXMLGetter(&xmldoc).FirstChild("material").FirstChild("group");
    for (; xGroup.ToNode(); xGroup = xGroup.NextSibling())
    {
        GroupData* orgGroupDef=mat->getGroup(xGroup.GetAttributeString("name","none"));
        GroupData* destGroup=orgGroupDef ? orgGroupDef : mat->addGroup();
        GroupDefinition* groupDef=&destGroup->groupDef_;

        if (xGroup.GetAttribute("name") || !orgGroupDef) {
            hStrCopy(groupDef->groupName, MATERIAL_STRING_MAX_LEN, xGroup.GetAttributeString("name","none"));
        }

        hXMLGetter xTech = xGroup.FirstChild("technique");
        for (; xTech.ToNode(); xTech = xTech.NextSibling())
        {
            TechniqueData* orgTechDef=destGroup->getTechnique(xTech.GetAttributeString("name","none"));
            TechniqueData* destTech=orgGroupDef ? orgTechDef : destGroup->addTechnique();
            TechniqueDefinition* techDef=&destTech->techDef_;

            if (xTech.GetAttribute("name") || !orgTechDef) {
                hStrCopy(techDef->technqiueName, MATERIAL_STRING_MAX_LEN, xTech.GetAttributeString("name","none"));
            }
            if (xTech.FirstChild("sort").ToNode() || !orgTechDef) {
                techDef->transparent = hStrICmp(xTech.FirstChild("sort").GetValueString("false"), "true") == 0;
            }
            if (xTech.FirstChild("layer").ToNode() || !orgTechDef) {
                techDef->layer = (hByte)(xTech.FirstChild("layer").GetValueInt()&0xFF);
            }

            hXMLGetter xPass = xTech.FirstChild("pass");
            for (hUint passIdx=0; xPass.ToNode(); xPass = xPass.NextSibling(), ++passIdx)
            {
                hBool newpass=passIdx >= destTech->passes_.GetSize();
                PassDefintion newPassDef = {0};
                PassDefintion& passDef= newpass ? newPassDef : destTech->passes_[passIdx];

                if (xPass.FirstChild("blendenable").ToNode() || newpass) {
                    passDef.blendState.blendEnable_ = xPass.FirstChild("blendenable").GetValueEnum(g_trueFalseEnum, RSV_DISABLE);
                }
                if (xPass.FirstChild("blendop").ToNode() || newpass) {
                    passDef.blendState.blendOp_ = xPass.FirstChild("blendop").GetValueEnum(g_blendFuncEnum, RSV_BLEND_FUNC_ADD);
                }
                if (xPass.FirstChild("blendopalpha").ToNode() || newpass) {
                    passDef.blendState.blendOpAlpha_ = xPass.FirstChild("blendopalpha").GetValueEnum(g_blendFuncEnum, RSV_BLEND_FUNC_ADD);
                }
                if (xPass.FirstChild("destblend").ToNode() || newpass) {
                    passDef.blendState.destBlend_ = xPass.FirstChild("destblend").GetValueEnum(g_blendOpEnum, RSV_BLEND_OP_ONE);
                }
                if (xPass.FirstChild("destblendalpha").ToNode() || newpass) {
                    passDef.blendState.destBlendAlpha_ = xPass.FirstChild("destblendalpha").GetValueEnum(g_blendOpEnum, RSV_BLEND_OP_ONE);
                }
                if (xPass.FirstChild("srcblend").ToNode() || newpass) {
                    passDef.blendState.srcBlend_= xPass.FirstChild("srcblend").GetValueEnum(g_blendOpEnum, RSV_BLEND_OP_ONE);
                }
                if (xPass.FirstChild("srcblendalpha").ToNode() || newpass) {
                    passDef.blendState.srcBlendAlpha_= xPass.FirstChild("srcblendalpha").GetValueEnum(g_blendOpEnum, RSV_BLEND_OP_ONE);
                }
                if (xPass.FirstChild("rendertargetwritemask").ToNode() || newpass) {
                    passDef.blendState.renderTargetWriteMask_ = xPass.FirstChild("rendertargetwritemask").GetValueHex(0xFF);
                }

                if (xPass.FirstChild("depthtest").ToNode() || newpass) {
                    passDef.depthState.depthEnable_ = xPass.FirstChild("depthtest").GetValueEnum(g_trueFalseEnum, RSV_DISABLE);
                }
                if (xPass.FirstChild("depthfunc").ToNode() || newpass) {
                    passDef.depthState.depthFunc_ = xPass.FirstChild("depthfunc").GetValueEnum(g_depthEnum, RSV_Z_CMP_LESS);
                }
                if (xPass.FirstChild("depthwrite").ToNode() || newpass) {
                    passDef.depthState.depthWriteMask_ = xPass.FirstChild("depthwrite").GetValueEnum(g_trueFalseEnum, RSV_DISABLE);
                }
                if (xPass.FirstChild("stencilenable").ToNode() || newpass) {
                    passDef.depthState.stencilEnable_ = xPass.FirstChild("stencilenable").GetValueEnum(g_trueFalseEnum, RSV_DISABLE);
                }
                if (xPass.FirstChild("stencilfunc").ToNode() || newpass) {
                    passDef.depthState.stencilFunc_ = xPass.FirstChild("stencilfunc").GetValueEnum(g_stencilFuncEnum, RSV_SF_CMP_NEVER);
                }
                if (xPass.FirstChild("stencildepthfailop").ToNode() || newpass) {
                    passDef.depthState.stencilDepthFailOp_ = xPass.FirstChild("stencildepthfailop").GetValueEnum(g_stencilOpEnum, RSV_SO_KEEP);
                }
                if (xPass.FirstChild("stencilfail").ToNode() || newpass) {
                    passDef.depthState.stencilFailOp_ = xPass.FirstChild("stencilfail").GetValueEnum(g_stencilOpEnum, RSV_SO_KEEP);
                }
                if (xPass.FirstChild("stencilpass").ToNode() || newpass) {
                    passDef.depthState.stencilPassOp_ = xPass.FirstChild("stencilpass").GetValueEnum(g_stencilOpEnum, RSV_SO_KEEP);
                }
                if (xPass.FirstChild("stencilreadmask").ToNode() || newpass) {
                    passDef.depthState.stencilReadMask_ = xPass.FirstChild("stencilreadmask").GetValueHex(0xFFFFFFFF);
                }
                if (xPass.FirstChild("stencilwritemask").ToNode() || newpass) {
                    passDef.depthState.stencilWriteMask_ = xPass.FirstChild("stencilwritemask").GetValueHex(0xFFFFFFFF);
                }
                if (xPass.FirstChild("stencilref").ToNode() || newpass) {
                    passDef.depthState.stencilRef_ = xPass.FirstChild("stencilref").GetValueHex(0x00000000);
                }

                if (xPass.FirstChild("cullmode").ToNode() || newpass) {
                    passDef.rasterizerState.cullMode_ = xPass.FirstChild("cullmode").GetValueEnum(g_cullModeEnum, RSV_CULL_MODE_CCW);
                }
                passDef.rasterizerState.depthBias_ = 0;//xPass.FirstChild("depthbias").GetValueFloat();
                if (xPass.FirstChild("depthbiasclamp").ToNode() || newpass) {
                    passDef.rasterizerState.depthBiasClamp_ = xPass.FirstChild("depthbiasclamp").GetValueFloat();
                }
                if (xPass.FirstChild("depthclipenable").ToNode() || newpass) {
                    passDef.rasterizerState.depthClipEnable_ = xPass.FirstChild("depthclipenable").GetValueEnum(g_trueFalseEnum, RSV_DISABLE);
                }
                if (xPass.FirstChild("fillmode").ToNode() || newpass) {
                    passDef.rasterizerState.fillMode_ = xPass.FirstChild("fillmode").GetValueEnum(g_fillModeEnum, RSV_FILL_MODE_SOLID);
                }
                if (xPass.FirstChild("slopescaleddepthbias").ToNode() || newpass) {
                    passDef.rasterizerState.slopeScaledDepthBias_ = xPass.FirstChild("slopescaleddepthbias").GetValueFloat();
                }
                if (xPass.FirstChild("scissortest").ToNode() || newpass) {
                    passDef.rasterizerState.scissorEnable_ = xPass.FirstChild("scissortest").GetValueEnum(g_trueFalseEnum, RSV_DISABLE);
                }
                passDef.rasterizerState.frontCounterClockwise_ = RSV_ENABLE;

                if (xPass.FirstChild("vertex").ToNode() || newpass) {
                    passDef.vertexProgramID   = hResourceManager::BuildResourceID(xPass.FirstChild("vertex").GetValueString());
                }
                if (xPass.FirstChild("fragment").ToNode() || newpass) {
                    passDef.fragmentProgramID = hResourceManager::BuildResourceID(xPass.FirstChild("fragment").GetValueString());
                }
                if (xPass.FirstChild("geometry").ToNode() || newpass) {
                    passDef.geometryProgramID = hResourceManager::BuildResourceID(xPass.FirstChild("geometry").GetValueString());
                }
                if (xPass.FirstChild("hull").ToNode() || newpass) {
                    passDef.hullProgramID = hResourceManager::BuildResourceID(xPass.FirstChild("hull").GetValueString());
                }
                if (xPass.FirstChild("domain").ToNode() || newpass) {
                    passDef.domainProgramID = hResourceManager::BuildResourceID(xPass.FirstChild("domain").GetValueString());
                }

                if (newpass)  {
                    destTech->passes_.PushBack(passDef);
                }
            }
        }
    }
}
