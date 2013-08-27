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
#include <boost/smart_ptr.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <vector>
#include <string>

typedef std::vector< std::string > StrVectorType;

class TechniqueData
{
public:
    TechniqueData()
    {
    }
public:
    typedef std::vector< PassDefintion > PassArrayType;

    TechniqueDefinition     techDef_;
    PassArrayType           passes_;
};

class GroupData
{
public:
    GroupData()
    {}
public:
    typedef std::vector< TechniqueData > TechniqueListType;

    TechniqueData* addTechnique(){
        techniques_.push_back(TechniqueData());
        return &techniques_[techniques_.size()-1];
    }
    TechniqueData* getTechnique(const hChar* name) {
        for (hUint i=0, n=(hUint)techniques_.size(); i<n; ++i) {
            if (Heart::hStrCmp(name, techniques_[i].techDef_.technqiueName)==0) {
                return &techniques_[i];
            }
        }
        return hNullptr;
    }

    GroupDefinition         groupDef_;
    TechniqueListType       techniques_;
};

class MaterialData
{
public:
    MaterialData()
    {}
public:
    typedef std::vector< SamplerDefinition >    SamplerArrayType;
    typedef std::vector< ParameterDefinition >  ParamArrayType;
    typedef std::vector< GroupData >            GroupListType;

    GroupData* addGroup() {
        groups_.push_back(GroupData());
        return &groups_[groups_.size()-1];
    }
    GroupData* getGroup(const hChar* name) {
        for (hUint i=0, n=(hUint)groups_.size(); i<n; ++i) {
            if (Heart::hStrCmp(name, groups_[i].groupDef_.groupName)==0) {
                return &groups_[i];
            }
        }
        return NULL;
    }
    SamplerDefinition* getSamplerByName(const hChar* name) {
        for (hUint i=0,n=(hUint)samplers_.size(); i<n; ++i) {
            if (Heart::hStrCmp(name, samplers_[i].samplerName)==0) {
                return &samplers_[i];
            }
        }
        return NULL;
    }
    ParameterDefinition* getParameterByName(const hChar* name) {
        for (hUint i=0,n=(hUint)parameters_.size(); i<n; ++i) {
            if (Heart::hStrCmp(name, parameters_[i].parameterName)==0) {
                return &parameters_[i];
            }
        }
        return NULL;
    }

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
    {"float",   Heart::ePTFloat },
    {"int",     Heart::ePTInt   },
    {"colour",  Heart::ePTColour},
    {"texture", Heart::ePTTexture},
    {"none",    Heart::ePTNone  },
};


Heart::hXMLEnumReamp g_samplerStates[] =
{
    { "repeat",                  Heart::SSV_WRAP },
    { "wrap",                    Heart::SSV_WRAP },

    { "clamp",                   Heart::SSV_CLAMP },
    { "clamptoedge",             Heart::SSV_CLAMP },
    { "mirror",                  Heart::SSV_MIRROR },
    { "border",                  Heart::SSV_BORDER },
    { "aniso",                   Heart::SSV_ANISOTROPIC },
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

void readMaterialXMLToMaterialData(const rapidxml::xml_document<>& xmldoc, const hChar* xmlpath, MaterialData* mat, StrVectorType* includes, StrVectorType* deps);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int MB_API materialCompile(lua_State* L) {
    using namespace Heart;

    /* Args from Lua (1: input files table, 2: dep files table, 3: parameter table, 4: outputpath)*/
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TTABLE);
    luaL_checktype(L, 3, LUA_TTABLE);
    luaL_checktype(L, 4, LUA_TSTRING);

    std::vector<std::string> openedfiles;
    MaterialHeader matHeader = {0};
    MaterialData matData;
    boost::system::error_code ec;
    std::string filepath;
    rapidxml::xml_document<> xmldoc;
    hUint filesize=0;
    StrVectorType depresnames;
    StrVectorType includes;

    lua_getglobal(L, "buildpathresolve");
    lua_rawgeti(L, 1, 1);
    if(!lua_isstring(L, -1)) {
        luaL_error(L, "input path is not a string");
        return 0;
    }
    lua_call(L, 1, 1);
    filepath=lua_tostring(L, -1);
    lua_pop(L, 1);

    filesize=(hUint)boost::filesystem::file_size(filepath.c_str(), ec);
    if (ec) {
        luaL_error(L, "Unable to get file size for file %s", filepath.c_str());
        return 0;
    }

    boost::shared_array<hChar> xmlmem = boost::shared_array<hChar>(new hChar[filesize+1]);
    std::ifstream infile;
    infile.open(filepath);
    if (!infile.is_open()) {
        luaL_error(L, "Unable to open file %s", filepath.c_str());
        return 0;
    }
    memset(xmlmem.get(), 0, filesize+1);
    infile.read(xmlmem.get(), filesize);
    infile.close();
    openedfiles.push_back(filepath);
    try {
        xmldoc.parse< rapidxml::parse_default >(xmlmem.get());
    } catch (...) {
        luaL_error(L, "Failed to parse material file");
        return 0;
    }

    readMaterialXMLToMaterialData(xmldoc, filepath.c_str(), &matData, &includes, &depresnames);

    for (auto i=includes.begin(),n=includes.end(); i!=n; ++i) {
        openedfiles.push_back(*i);
    }

    matData.header_.samplerCount=(hByte)matData.samplers_.size();
    matData.header_.samplerOffset=0;
    matData.header_.parameterCount=(hUint16)matData.parameters_.size();
    matData.header_.parameterOffset=0;
    matData.header_.groupCount=(hByte)matData.groups_.size();
    matData.header_.groupOffset=0;

    if (matData.header_.samplerCount != matData.samplers_.size()) {
        luaL_error(L, "sampler count is too large (greater than 255)");
        return 0;
    }
    if (matData.header_.groupCount != matData.groups_.size()) {
        luaL_error(L, "sampler count is too large (greater than 255)");
        return 0;
    }
    if (matData.header_.parameterCount != matData.parameters_.size()) {
        luaL_error(L, "parameter count is too large (greater than 65535)");
        return 0;
    }

    lua_getglobal(L, "buildpathresolve");
    lua_pushvalue(L, 4);
    lua_call(L, 1, 1);
    const hChar* outputpath=lua_tostring(L, -1);
    std::ofstream output;
    output.open(outputpath, std::ios_base::out|std::ios_base::binary);

    if (!output.is_open()) {
        luaL_error(L, "Unable to open output file %s for writing", outputpath);
        return 0;
    }

    output.write((char*)&matData.header_, sizeof(matData.header_));

    for (hUint samp=0,sampn=matData.header_.samplerCount; samp<sampn; ++samp) {
        output.write((char*)&matData.samplers_[samp], sizeof(SamplerDefinition));
    }

    for (hUint param=0, paramn=matData.header_.parameterCount; param<paramn; ++param) {
        output.write((char*)&matData.parameters_[param], sizeof(ParameterDefinition));
    }

    for (hUint grp=0, grpn=(hUint)matData.groups_.size(); grp<grpn; ++grp) {
        matData.groups_[grp].groupDef_.techniques=(hUint16)matData.groups_[grp].techniques_.size();
        output.write((char*)&matData.groups_[grp].groupDef_, sizeof(matData.groups_[grp].groupDef_));
        for (hUint tech=0, techn=(hUint)matData.groups_[grp].techniques_.size(); tech<techn; ++tech) {
            matData.groups_[grp].techniques_[tech].techDef_.passes=(hUint16)matData.groups_[grp].techniques_[tech].passes_.size();
            output.write((char*)&matData.groups_[grp].techniques_[tech].techDef_, sizeof(matData.groups_[grp].techniques_[tech].techDef_));
            for (hUint passidx=0,passidxn=(hUint)matData.groups_[grp].techniques_[tech].passes_.size(); passidx<passidxn; ++passidx) {
                output.write((char*)&matData.groups_[grp].techniques_[tech].passes_[passidx], sizeof(PassDefintion));
            }
        }
    }

    output.close();

    //Return a list of resources this material is dependent on
    lua_newtable(L);
    hUint idx=1;
    for (auto i=depresnames.begin(),n=depresnames.end(); i!=n; ++i) {
        lua_pushstring(L, i->c_str());
        lua_rawseti(L, -2, idx);
        ++idx;
    }

    lua_newtable(L); // push table of input files we depend on (absolute paths)
    idx=1;
    for (auto i=openedfiles.begin(), n=openedfiles.end(); i!=n; ++i) {
        lua_pushstring(L, i->c_str());
        lua_rawseti(L, -2, idx);
        ++idx;
    }
    return 2;
}

int MB_API materialScanIncludes(lua_State* L) {
    /* Args from Lua (1: input file)*/
    luaL_checktype(L, 1, LUA_TSTRING);

    MaterialHeader matHeader = {0};
    MaterialData matData;
    boost::system::error_code ec;
    const hChar* filepath=hNullptr;
    rapidxml::xml_document<> xmldoc;
    hUint filesize=0;
    StrVectorType depresnames;
    StrVectorType includes;

    lua_getglobal(L, "buildpathresolve");
    lua_pushvalue(L, 1);
    lua_call(L, 1, 1);
    filepath=lua_tostring(L, -1);
    lua_pop(L, 1);

    filesize=(hUint)boost::filesystem::file_size(filepath, ec);
    if (ec) {
        luaL_error(L, "Unable to get file size for file %s", filepath);
        return 0;
    }

    boost::shared_array<hChar> xmlmem = boost::shared_array<hChar>(new hChar[filesize+1]);
    std::ifstream infile;
    infile.open(filepath);
    if (!infile.is_open()) {
        luaL_error(L, "Unable to open file %s", filepath);
        return 0;
    }
    memset(xmlmem.get(), 0, filesize+1);
    infile.read(xmlmem.get(), filesize);
    infile.close();

    try {
        xmldoc.parse< rapidxml::parse_default >(xmlmem.get());
    } catch (...) {
        luaL_error(L, "Failed to parse material file");
        return 0;
    }

    readMaterialXMLToMaterialData(xmldoc, filepath, &matData, &includes, &depresnames);

    //Return a list of resources this material is dependent on
    lua_newtable(L);
    hUint idx=1;
    for (StrVectorType::iterator i=includes.begin(),n=includes.end(); i!=n; ++i) {
        lua_pushstring(L, i->c_str());
        lua_rawseti(L, -2, idx);
        ++idx;
    }

    // return 1 list
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void readMaterialXMLToMaterialData(const rapidxml::xml_document<>& xmldoc, const hChar* xmlpath, MaterialData* mat, StrVectorType* includes, StrVectorType* deps) {
    using namespace Heart;

    mat->header_.resHeader.resourceType = hMAKE_FOURCC('X','X','X','X');//Write a invalid four cc, we'll write a correct one later
    mat->header_.version = MATERIAL_VERSION;
    mat->header_.resHeader.resourceType = MATERIAL_MAGIC_NUM;
    strcpy_s(mat->header_.defaultGroupName, MATERIAL_STRING_MAX_LEN, hXMLGetter(&xmldoc).FirstChild("material").FirstChild("defaultgroup").GetValueString("lowdetail"));

    if (hXMLGetter(&xmldoc).FirstChild("material").GetAttributeString("inherit")) {
        //Load the base first
        const hChar* basepathrel=hXMLGetter(&xmldoc).FirstChild("material").GetAttributeString("inherit");
        hUint pathlen=(hUint)strlen(basepathrel)+(hUint)strlen(xmlpath)+1;//+1 for NULL
        hChar* fullbasepath=(hChar*)hAlloca(pathlen);
        hChar* pathSep=NULL;
        strcpy_s(fullbasepath, pathlen, xmlpath);
        pathSep=strrchr(fullbasepath, '/');
        if (pathSep) {
            *(pathSep+1)=0;
            strcat_s(fullbasepath, pathlen, basepathrel);
        } else {
            strcpy_s(fullbasepath, pathlen, basepathrel);
        }
        boost::shared_array<hChar> ptr;
        std::ifstream incfile;
        boost::system::error_code ec;
        hUint incfilesize=(hUint)boost::filesystem::file_size(fullbasepath, ec);
        incfile.open(fullbasepath);
        if (incfile.is_open()) {
            rapidxml::xml_document<> xmldocbase;
            ptr=boost::shared_array<hChar>(new hChar[incfilesize+1]);
            memset(ptr.get(), 0, incfilesize);
            incfile.read(ptr.get(), incfilesize);
            try {
                xmldocbase.parse< rapidxml::parse_default >(ptr.get());
            } catch (...){
                return;
            }
            incfile.close();
            includes->push_back(fullbasepath);
            readMaterialXMLToMaterialData(xmldocbase, fullbasepath, mat, includes, deps);
        }
    }

    hXMLGetter xSampler = hXMLGetter(&xmldoc).FirstChild("material").FirstChild("sampler");
    for (; xSampler.ToNode(); xSampler = xSampler.NextSibling())
    {
        SamplerDefinition newSampDef = {0};
        SamplerDefinition* orgSampDef=mat->getSamplerByName(xSampler.GetAttributeString("name","none"));
        SamplerDefinition* sampDef= orgSampDef ? orgSampDef : &newSampDef;

        if (xSampler.GetAttribute("name") || !orgSampDef) {
            strcpy_s(sampDef->samplerName, MATERIAL_STRING_MAX_LEN, xSampler.GetAttributeString("name","none"));
        }
        //if (xSampler.FirstChild("texture").ToNode() || !orgSampDef) {
        //    sampDef->defaultTextureID = hResourceManager::BuildResourceID(xSampler.FirstChild("texture").GetValueString());
        //}
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
            mat->samplers_.push_back(*sampDef);
        }
    }

    hXMLGetter xParameter = hXMLGetter(&xmldoc).FirstChild("material").FirstChild("parameter");
    for (; xParameter.ToNode(); xParameter = xParameter.NextSibling())
    {
        ParameterDefinition newParamDef = {0};
        ParameterDefinition* orgParamDef=mat->getParameterByName(xParameter.GetAttributeString("name","none"));
        ParameterDefinition* paramDef=orgParamDef ? orgParamDef : &newParamDef;

        if (xParameter.GetAttribute("name") || !orgParamDef) {
            strcpy_s(paramDef->parameterName, MATERIAL_PARAM_STRING_MAX_LEN, xParameter.GetAttributeString("name","none"));
        }
        if (xParameter.GetAttribute("type") || !orgParamDef) {
            paramDef->type = xParameter.GetAttributeEnum("type", g_parameterTypes, ePTNone );
        }
        if (xParameter.GetValueString() || !orgParamDef) {
            const hChar* valstr=xParameter.GetValueString();
            if (paramDef->type == ePTFloat) {
                hFloat* fd=paramDef->floatData;
                paramDef->count=sscanf_s(valstr, " %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f ",
                    fd, fd+1, fd+2, fd+3, fd+4, fd+5, fd+6, fd+7, fd+8, fd+9, fd+10, fd+11, fd+12, fd+13, fd+14, fd+15);
            } else if (paramDef->type == ePTInt) {
                hInt* id=paramDef->intData;
                paramDef->count=sscanf_s(valstr, " %d , %d , %d , %d , %d , %d , %d , %d , %d , %d , %d , %d , %d , %d , %d , %d ",
                    id, id+1, id+2, id+3, id+4, id+5, id+6, id+7, id+8, id+9, id+10, id+11, id+12, id+13, id+14, id+15);
            } else if (paramDef->type == ePTColour) {
                hFloat* fd=paramDef->colourData;
                paramDef->count=sscanf_s(valstr, " %f , %f , %f , %f ",
                    fd, fd+1, fd+2, fd+3);
            } else if (paramDef->type == ePTTexture) {
                paramDef->count=1;
                paramDef->resourceID=hResourceManager::BuildResourceID(valstr);
            }
            paramDef->count*=4;
        }

        if (!orgParamDef) {
            mat->parameters_.push_back(newParamDef);
        }
    }

    hXMLGetter xGroup = hXMLGetter(&xmldoc).FirstChild("material").FirstChild("group");
    for (; xGroup.ToNode(); xGroup = xGroup.NextSibling())
    {
        GroupData* orgGroupDef=mat->getGroup(xGroup.GetAttributeString("name","none"));
        GroupData* destGroup=orgGroupDef ? orgGroupDef : mat->addGroup();
        GroupDefinition* groupDef=&destGroup->groupDef_;

        if (xGroup.GetAttribute("name") || !orgGroupDef) {
            strcpy_s(groupDef->groupName, MATERIAL_STRING_MAX_LEN, xGroup.GetAttributeString("name","none"));
        }

        hXMLGetter xTech = xGroup.FirstChild("technique");
        for (; xTech.ToNode(); xTech = xTech.NextSibling())
        {
            TechniqueData* orgTechDef=destGroup->getTechnique(xTech.GetAttributeString("name","none"));
            TechniqueData* destTech=orgGroupDef ? orgTechDef : destGroup->addTechnique();
            TechniqueDefinition* techDef=&destTech->techDef_;

            if (xTech.GetAttribute("name") || !orgTechDef) {
                strcpy_s(techDef->technqiueName, MATERIAL_STRING_MAX_LEN, xTech.GetAttributeString("name","none"));
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
                hBool newpass=passIdx >= destTech->passes_.size();
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
                    if (passDef.vertexProgramID) {
                        deps->push_back(xPass.FirstChild("vertex").GetValueString());
                    }
                }
                if (xPass.FirstChild("fragment").ToNode() || newpass) {
                    passDef.fragmentProgramID = hResourceManager::BuildResourceID(xPass.FirstChild("fragment").GetValueString());
                    if (passDef.fragmentProgramID) {
                        deps->push_back(xPass.FirstChild("fragment").GetValueString());
                    }
                }
                if (xPass.FirstChild("geometry").ToNode() || newpass) {
                    passDef.geometryProgramID = hResourceManager::BuildResourceID(xPass.FirstChild("geometry").GetValueString());
                    if (passDef.geometryProgramID) {
                        deps->push_back(xPass.FirstChild("geometry").GetValueString());
                    }
                }
                if (xPass.FirstChild("hull").ToNode() || newpass) {
                    passDef.hullProgramID = hResourceManager::BuildResourceID(xPass.FirstChild("hull").GetValueString());
                    if (passDef.hullProgramID) {
                        deps->push_back(xPass.FirstChild("hull").GetValueString());
                    }
                }
                if (xPass.FirstChild("domain").ToNode() || newpass) {
                    passDef.domainProgramID = hResourceManager::BuildResourceID(xPass.FirstChild("domain").GetValueString());
                    if (passDef.domainProgramID) {
                        deps->push_back(xPass.FirstChild("domain").GetValueString());
                    }
                }

                if (newpass)  {
                    destTech->passes_.push_back(passDef);
                }
            }
        }
    }
}

extern "C" {
//Lua entry point calls
DLL_EXPORT int MB_API luaopen_material(lua_State *L) {
    static const luaL_Reg materiallib[] = {
        {"compile"      , materialCompile},
        {"scanincludes" , materialScanIncludes},
        {NULL, NULL}
    };
    luaL_newlib(L, materiallib);
    return 1;
}
}

