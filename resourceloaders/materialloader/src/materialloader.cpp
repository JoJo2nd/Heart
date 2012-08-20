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
// Enum Tables ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
Heart::hXMLEnumReamp g_samplerStates[] =
{
    { "repeat",                  Heart::SSV_WRAP },
    { "wrap",                    Heart::SSV_WRAP },
    { "clamp",                   Heart::SSV_CLAMP },
    { "clamptoedge",             Heart::SSV_CLAMP },
    { "mirror",                  Heart::SSV_MIRROR },
    { "border",                  Heart::SSV_BORDER },
    { "anisotropic",             Heart::SSV_ANISOTROPIC },
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

DLL_EXPORT
Heart::hResourceClassBase* HEART_API HeartBinLoader( Heart::hISerialiseStream* inFile, Heart::hIDataParameterSet*, Heart::HeartEngine* )
{
    using namespace Heart;

    hMaterial* material = hNEW(GetGlobalHeap(), hMaterial)();

    Heart::hSerialiser ser;
    ser.Deserialise(inFile, *material);

    return material;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
Heart::hResourceClassBase* HEART_API HeartRawLoader( Heart::hIDataCacheFile* inFile, Heart::hIBuiltDataCache* fileCache, Heart::hIDataParameterSet* params, Heart::HeartEngine* engine, Heart::hISerialiseStream* binoutput )
{
    using namespace Heart;

    hXMLDocument xmldoc;
    hChar* xmlmem = (hChar*)hHeapMalloc(GetGlobalHeap(), inFile->Lenght()+1);
    inFile->Read(xmlmem, inFile->Lenght());
    xmlmem[inFile->Lenght()] = 0;

    if (xmldoc.ParseSafe< rapidxml::parse_default >(xmlmem, GetGlobalHeap()) == hFalse)
        return NULL;

    hMaterial* material = hNEW(GetGlobalHeap(), hMaterial)();

    hXMLGetter xSampler = hXMLGetter(&xmldoc).FirstChild("material").FirstChild("sampler");
    for (; xSampler.ToNode(); xSampler = xSampler.NextSibling())
    {
        hSamplerParameter samp;
        hStrCopy(samp.name_, 32, xSampler.GetAttributeString("name","none"));
        samp.nameLen_ = hStrLen(samp.name_);
        samp.defaultTextureID_ = hResourceManager::BuildResourceID(xSampler.FirstChild("texture").GetValueString());
        samp.samplerDesc_.addressU_ = xSampler.FirstChild("addressu").GetValueEnum(g_samplerStates, SSV_CLAMP);
        samp.samplerDesc_.addressV_ = xSampler.FirstChild("addressv").GetValueEnum(g_samplerStates, SSV_CLAMP);
        samp.samplerDesc_.addressW_ = xSampler.FirstChild("addressw").GetValueEnum(g_samplerStates, SSV_CLAMP);
        samp.samplerDesc_.borderColour_ = xSampler.FirstChild("bordercolour").GetValueColour(hColour(0.f, 0.f, 0.f, 1.f));
        samp.samplerDesc_.filter_   = xSampler.FirstChild("filter").GetValueEnum(g_samplerStates, SSV_POINT);
        samp.samplerDesc_.maxAnisotropy_ = xSampler.FirstChild("maxanisotropy").GetValueInt(1);
        samp.samplerDesc_.minLOD_ = xSampler.FirstChild("minlod").GetValueFloat();
        samp.samplerDesc_.maxLOD_ = xSampler.FirstChild("maxlod").GetValueFloat(FLT_MAX);
        samp.samplerDesc_.mipLODBias_ = xSampler.FirstChild("miplodbias").GetValueFloat();

        material->AddSamplerParameter(samp);
    }

    hXMLGetter xGroup = hXMLGetter(&xmldoc).FirstChild("material").FirstChild("group");
    for (; xGroup.ToNode(); xGroup = xGroup.NextSibling())
    {
        hMaterialGroup* group = material->AddGroup(xGroup.GetAttributeString("name","none"));

        hXMLGetter xTech = xGroup.FirstChild("technique");
        for (; xTech.ToNode(); xTech = xTech.NextSibling())
        {
            hMaterialTechnique tech;
            tech.SetName(xTech.GetAttributeString("name","none"));
            tech.SetSortAsTransparent(hStrICmp(xTech.FirstChild("sort").GetValueString("false"), "true") == 0);
            tech.SetLayer((hByte)(xTech.FirstChild("layer").GetValueInt()&0xFF));


            hXMLGetter xPass = xTech.FirstChild("pass");
            for (; xPass.ToNode(); xPass = xPass.NextSibling())
            {
                hMaterialTechniquePass pass;
                hBlendStateDesc blendstate;
                hDepthStencilStateDesc depthstate;
                hRasterizerStateDesc rasstate;

                blendstate.blendEnable_ = xPass.FirstChild("blendenable").GetValueEnum(g_trueFalseEnum, RSV_DISABLE);
                blendstate.blendOp_ = xPass.FirstChild("blendop").GetValueEnum(g_blendFuncEnum, RSV_BLEND_FUNC_ADD);
                blendstate.blendOpAlpha_ = xPass.FirstChild("blendopalpha").GetValueEnum(g_blendFuncEnum, RSV_BLEND_FUNC_ADD);
                blendstate.destBlend_ = xPass.FirstChild("destblend").GetValueEnum(g_blendOpEnum, RSV_BLEND_OP_ONE);
                blendstate.destBlendAlpha_ = xPass.FirstChild("destblendalpha").GetValueEnum(g_blendOpEnum, RSV_BLEND_OP_ONE);
                blendstate.srcBlend_= xPass.FirstChild("srcblend").GetValueEnum(g_blendOpEnum, RSV_BLEND_OP_ONE);
                blendstate.srcBlendAlpha_= xPass.FirstChild("srcblendalpha").GetValueEnum(g_blendOpEnum, RSV_BLEND_OP_ONE);
                blendstate.renderTargetWriteMask_ = xPass.FirstChild("rendertargetwritemask").GetValueHex(0xFF);

                depthstate.depthEnable_ = xPass.FirstChild("depthtest").GetValueEnum(g_trueFalseEnum, RSV_DISABLE);
                depthstate.depthFunc_ = xPass.FirstChild("depthfunc").GetValueEnum(g_depthEnum, RSV_Z_CMP_LESS);
                depthstate.depthWriteMask_ = xPass.FirstChild("depthwrite").GetValueEnum(g_trueFalseEnum, RSV_DISABLE);
                depthstate.stencilEnable_ = xPass.FirstChild("stencilenable").GetValueEnum(g_trueFalseEnum, RSV_DISABLE);
                depthstate.stencilFunc_ = xPass.FirstChild("stencilfunc").GetValueEnum(g_stencilFuncEnum, RSV_SF_CMP_NEVER);
                depthstate.stencilDepthFailOp_ = xPass.FirstChild("stencildepthfailop").GetValueEnum(g_stencilOpEnum, RSV_SO_KEEP);
                depthstate.stencilFailOp_ = xPass.FirstChild("stencilfail").GetValueEnum(g_stencilOpEnum, RSV_SO_KEEP);
                depthstate.stencilPassOp_ = xPass.FirstChild("stencilpass").GetValueEnum(g_stencilOpEnum, RSV_SO_KEEP);
                depthstate.stencilReadMask_ = xPass.FirstChild("stencilreadmask").GetValueHex(0xFFFFFFFF);
                depthstate.stencilWriteMask_ = xPass.FirstChild("stencilwritemask").GetValueHex(0xFFFFFFFF);
                depthstate.stencilRef_ = xPass.FirstChild("stencilref").GetValueHex(0x00000000);

                rasstate.cullMode_ = xPass.FirstChild("cullmode").GetValueEnum(g_cullModeEnum, RSV_CULL_MODE_CCW);
                rasstate.depthBias_ = xPass.FirstChild("depthbias").GetValueFloat();
                rasstate.depthBiasClamp_ = xPass.FirstChild("depthbiasclamp").GetValueFloat();
                rasstate.depthClipEnable_ = xPass.FirstChild("depthclipenable").GetValueEnum(g_trueFalseEnum, RSV_DISABLE);
                rasstate.fillMode_ = xPass.FirstChild("fillmode").GetValueEnum(g_fillModeEnum, RSV_FILL_MODE_SOLID);
                rasstate.slopeScaledDepthBias_ = xPass.FirstChild("slopescaleddepthbias").GetValueFloat();
                rasstate.scissorEnable_ = xPass.FirstChild("scissortest").GetValueEnum(g_trueFalseEnum, RSV_DISABLE);
                rasstate.frontCounterClockwise_ = RSV_ENABLE;

                pass.SetBlendStateDesc(blendstate);
                pass.SetDepthStencilStateDesc(depthstate);
                pass.SetRasterizerStateDesc(rasstate);
                pass.SetVertexShaderResID(hResourceManager::BuildResourceID(xPass.FirstChild("vertex").GetValueString()));
                pass.SetFragmentShaderResID(hResourceManager::BuildResourceID(xPass.FirstChild("fragment").GetValueString()));

                tech.AppendPass(pass);
            }

            group->techniques_.PushBack(tech);
        }
    }

    Heart::hSerialiser ser;
    ser.Serialise(binoutput, *material);

    return material;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
hBool HEART_API HeartPackageLink( Heart::hResourceClassBase* resource, Heart::HeartEngine* engine )
{
    using namespace Heart;
    hMaterial* mat = static_cast< hMaterial* >(resource);
    return mat->Link(engine->GetResourceManager(), engine->GetRenderer(), engine->GetRenderer()->GetMaterialManager());
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
