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

#include <boost/smart_ptr.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <stdio.h>
#include "rapidxml/rapidxml.hpp"
#include "Heart.h" // TODO: remove this somehow

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

#if defined (material_builder_EXPORTS)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif

#define MB_API __cdecl

typedef std::vector< std::string > StrVectorType;

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

#define luaL_errorthrow(L, fmt, ...) \
    luaL_where(L, 1); \
    lua_pushfstring(L, fmt, __VA_ARGS__); \
    lua_concat(L, 2); \
    throw std::exception();

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void readMaterialXMLToMaterialData(lua_State* L, const rapidxml::xml_document<>& xmldoc, const hChar* xmlpath, Heart::proto::MaterialResource* mat, StrVectorType* includes, StrVectorType* deps);

Heart::proto::MaterialSampler* findOrAddMaterialSampler(Heart::proto::MaterialResource* mat, const hChar* name) {
    for (hUint i=0, n=mat->samplers_size(); i<n; ++i) {
        if (Heart::hStrCmp(mat->samplers(i).samplername().c_str(), name) == 0) {
            return mat->mutable_samplers()->Mutable(i);
        }
    }
    auto r=mat->add_samplers();
    r->set_samplername(name);
    return r;
}

Heart::proto::MaterialParameter* findOrAddMaterialParameter(Heart::proto::MaterialResource* mat, const hChar* name) {
    for (hUint i=0, n=mat->parameters_size(); i<n; ++i) {
        if (Heart::hStrCmp(mat->parameters(i).paramname().c_str(), name) == 0) {
            return mat->mutable_parameters()->Mutable(i);
        }
    }
    auto r = mat->add_parameters();
    r->set_paramname(name);
    return r;
}

Heart::proto::MaterialGroup* findOrAddMaterialGroup(Heart::proto::MaterialResource* mat, const hChar* name) {
    for (hUint i=0, n=mat->groups_size(); i<n; ++i) {
        if (Heart::hStrCmp(mat->groups(i).groupname().c_str(), name) == 0) {
            return mat->mutable_groups()->Mutable(i);
        }
    }
    auto r = mat->add_groups();
    r->set_groupname(name);
    return r;
}

Heart::proto::MaterialTechnique* findOrAddMaterialTechnique(Heart::proto::MaterialGroup* group, const hChar* name) {
    for (hUint i=0, n=group->technique_size(); i<n; ++i) {
        if (Heart::hStrCmp(group->technique(i).techniquename().c_str(), name) == 0) {
            return group->mutable_technique()->Mutable(i);
        }
    }
    auto r = group->add_technique();
    r->set_techniquename(name);
    return r;
}

Heart::proto::MaterialPass* findOrAddMaterialPass(Heart::proto::MaterialTechnique* tech, hUint passidx) {
    if (tech->passes_size() > (hInt)passidx) {
        return tech->mutable_passes(passidx);
    }
    return tech->add_passes();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int MB_API materialCompile(lua_State* L) {
    using namespace Heart;

    /* Args from Lua (1: input files table, 2: dep files table, 3: parameter table, 4: outputpath)*/
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TTABLE);
    luaL_checktype(L, 3, LUA_TTABLE);
    luaL_checktype(L, 4, LUA_TSTRING);
try {
    Heart::proto::MaterialResource materialresource;
    std::vector<std::string> openedfiles;
    boost::system::error_code ec;
    std::string filepath;
    rapidxml::xml_document<> xmldoc;
    hUint filesize=0;
    StrVectorType depresnames;
    StrVectorType includes;

    filepath=lua_tostring(L, 1);
    filesize=(hUint)boost::filesystem::file_size(filepath.c_str(), ec);
    if (ec) {
        luaL_errorthrow(L, "Unable to get file size for file %s", filepath.c_str());
    }

    boost::shared_array<hChar> xmlmem = boost::shared_array<hChar>(new hChar[filesize+1]);
    std::ifstream infile;
    infile.open(filepath);
    if (!infile.is_open()) {
        luaL_errorthrow(L, "Unable to open file %s", filepath.c_str());
    }
    memset(xmlmem.get(), 0, filesize+1);
    infile.read(xmlmem.get(), filesize);
    infile.close();
    openedfiles.push_back(filepath);
    try {
        xmldoc.parse< rapidxml::parse_default >(xmlmem.get());
    } catch (...) {
        luaL_errorthrow(L, "Failed to parse material file");
    }

    readMaterialXMLToMaterialData(L, xmldoc, filepath.c_str(), &materialresource, &includes, &depresnames);

    for (auto i=includes.begin(),n=includes.end(); i!=n; ++i) {
        openedfiles.push_back(*i);
    }

    const hChar* outputpath=lua_tostring(L, 4);
    std::ofstream output;
    output.open(outputpath, std::ios_base::out|std::ios_base::binary);

    if (!output.is_open()) {
        luaL_errorthrow(L, "Unable to open output file %s for writing", outputpath);
    }

    //write the resource header
    {
        google::protobuf::io::OstreamOutputStream filestream(&output);
        google::protobuf::io::CodedOutputStream outputstream(&filestream);
        Heart::proto::MessageContainer msgContainer;
        msgContainer.set_type_name(materialresource.GetTypeName());
        msgContainer.set_messagedata(materialresource.SerializeAsString());
        msgContainer.SerializePartialToCodedStream(&outputstream);
    }
    output.close();

    //Return a list of resources this material is dependent on
    lua_newtable(L); // push table of input files we depend on (absolute paths or relative to game data folder)
    int idx=1;
    for (auto i=depresnames.begin(),n=depresnames.end(); i!=n; ++i) {
        lua_pushstring(L, i->c_str());
        lua_rawseti(L, -2, idx);
        ++idx;
    }
    for (auto i=openedfiles.begin(), n=openedfiles.end(); i!=n; ++i) {
        lua_pushstring(L, i->c_str());
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

void readMaterialXMLToMaterialData(lua_State* L, const rapidxml::xml_document<>& xmldoc, const hChar* xmlpath, Heart::proto::MaterialResource* mat, StrVectorType* includes, StrVectorType* deps) {
    using namespace Heart;

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
            readMaterialXMLToMaterialData(L, xmldocbase, fullbasepath, mat, includes, deps);
        }
    }

    hXMLGetter xSampler = hXMLGetter(&xmldoc).FirstChild("material").FirstChild("sampler");
    for (; xSampler.ToNode(); xSampler = xSampler.NextSibling())
    {
        Heart::proto::MaterialSampler* sampler=hNullptr;
        if (xSampler.GetAttribute("name")) {
            sampler = findOrAddMaterialSampler(mat, xSampler.GetAttribute("name")->value());
        } else { // Error? can't process this!
            luaL_errorthrow(L, "Can't find name parameter for sampler in material");
        }

        if (xSampler.FirstChild("addressu").ToNode()) {
            sampler->mutable_samplerstate()->set_addressu((hUint)xSampler.FirstChild("addressu").GetValueEnum(g_samplerStates, SSV_CLAMP));
        }
        if (xSampler.FirstChild("addressv").ToNode()) {
            sampler->mutable_samplerstate()->set_addressv((hUint)xSampler.FirstChild("addressv").GetValueEnum(g_samplerStates, SSV_CLAMP));
        }
        if (xSampler.FirstChild("addressw").ToNode()) {
            sampler->mutable_samplerstate()->set_addressw((hUint)xSampler.FirstChild("addressw").GetValueEnum(g_samplerStates, SSV_CLAMP));
        }
        if (xSampler.FirstChild("bordercolour").ToNode()) {
            auto colour=xSampler.FirstChild("bordercolour").GetValueColour(hColour(0.f, 0.f, 0.f, 1.f));
            sampler->mutable_samplerstate()->mutable_bordercolour()->set_red((hUint)(colour.r_*255.f+.5f));
            sampler->mutable_samplerstate()->mutable_bordercolour()->set_green((hUint)(colour.g_*255.f+.5f));
            sampler->mutable_samplerstate()->mutable_bordercolour()->set_blue((hUint)(colour.b_*255.f+.5f));
            sampler->mutable_samplerstate()->mutable_bordercolour()->set_alpha((hUint)(colour.a_*255.f+.5f));
        }
        if (xSampler.FirstChild("filter").ToNode()) {
            sampler->mutable_samplerstate()->set_filter((hUint)xSampler.FirstChild("filter").GetValueEnum(g_samplerStates, SSV_POINT));
        }
        if (xSampler.FirstChild("maxanisotropy").ToNode()) {
            sampler->mutable_samplerstate()->set_maxanisotropy((hUint)xSampler.FirstChild("maxanisotropy").GetValueInt(1));
        }
        if (xSampler.FirstChild("minlod").ToNode()) {
            sampler->mutable_samplerstate()->set_minlod((hFloat)xSampler.FirstChild("minlod").GetValueFloat());
        }
        if (xSampler.FirstChild("maxlod").ToNode()) {
            sampler->mutable_samplerstate()->set_maxlod((hFloat)xSampler.FirstChild("maxlod").GetValueFloat(FLT_MAX));
        }
        if (xSampler.FirstChild("miplodbias").ToNode()) {
            sampler->mutable_samplerstate()->set_miplodbias((hFloat)xSampler.FirstChild("miplodbias").GetValueFloat());
        }
    }

    hXMLGetter xParameter = hXMLGetter(&xmldoc).FirstChild("material").FirstChild("parameter");
    for (; xParameter.ToNode(); xParameter = xParameter.NextSibling())
    {
        hParameterType ptype=ePTNone;
        proto::MaterialParameter* param=hNullptr;
        if (xParameter.GetAttribute("name")) {
            param=findOrAddMaterialParameter(mat, xParameter.GetAttributeString("name","none"));
        } else {
            luaL_errorthrow(L, "Can't find name for parameter in material");
        }
        if (xParameter.GetAttribute("type")) {
            ptype = xParameter.GetAttributeEnum("type", g_parameterTypes, ePTNone );
        }
        if (xParameter.GetValueString()) {
            const hChar* valstr=xParameter.GetValueString();
            if (ptype == ePTFloat) {
                hFloat fd[16];
                int count=sscanf_s(valstr, " %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f ",
                    fd, fd+1, fd+2, fd+3, fd+4, fd+5, fd+6, fd+7, fd+8, fd+9, fd+10, fd+11, fd+12, fd+13, fd+14, fd+15);
                for (int i=0; i<count; ++i) {
                    param->add_floatvalues(fd[i]);
                }
            } else if (ptype == ePTInt) {
                hInt id[16];
                int count=sscanf_s(valstr, " %d , %d , %d , %d , %d , %d , %d , %d , %d , %d , %d , %d , %d , %d , %d , %d ",
                    id, id+1, id+2, id+3, id+4, id+5, id+6, id+7, id+8, id+9, id+10, id+11, id+12, id+13, id+14, id+15);
                for (int i=0; i<count; ++i) {
                    param->add_intvalues(id[i]);
                }
            } else if (ptype == ePTColour) {
                hFloat fd[4] = {1.f, 1.f, 1.f, 1.f};
                int count=sscanf_s(valstr, " %f , %f , %f , %f ", fd, fd+1, fd+2, fd+3);
                if (count != 4) {
                    luaL_errorthrow(L, "Colour parsed have %d values, expected only 4", count);
                }
                auto colour=param->add_colourvalues();
                colour->set_red((hUint)(fd[0]*255.f+.5f));
                colour->set_green((hUint)(fd[1]*255.f+.5f));
                colour->set_blue((hUint)(fd[2]*255.f+.5f));
                colour->set_alpha((hUint)(fd[3]*255.f+.5f));
            } else if (ptype == ePTTexture) {
                param->set_resourceid(valstr);
            }
        }
    }

    hXMLGetter xGroup = hXMLGetter(&xmldoc).FirstChild("material").FirstChild("group");
    for (; xGroup.ToNode(); xGroup = xGroup.NextSibling())
    {
        Heart::proto::MaterialGroup* group=hNullptr;
        if (xGroup.GetAttribute("name")) {
            group=findOrAddMaterialGroup(mat, xGroup.GetAttributeString("name","none"));
        } else {
            luaL_errorthrow(L, "Material group is missing name");
        }

        hXMLGetter xTech = xGroup.FirstChild("technique");
        for (; xTech.ToNode(); xTech = xTech.NextSibling())
        {
            Heart::proto::MaterialTechnique* tech=hNullptr;
            if (xTech.GetAttribute("name")) {
                tech=findOrAddMaterialTechnique(group, xTech.GetAttributeString("name","none"));
            } else {
                luaL_errorthrow(L, "Material technique is missing name");
            }
            if (xTech.FirstChild("sort").ToNode()) {
                tech->set_transparent(hStrICmp(xTech.FirstChild("sort").GetValueString("false"), "true") == 0);
            }
            if (xTech.FirstChild("layer").ToNode()) {
                tech->set_layer(xTech.FirstChild("layer").GetValueInt()&0xFF);
            }

            hXMLGetter xPass = xTech.FirstChild("pass");
            for (hUint passIdx=0; xPass.ToNode(); xPass = xPass.NextSibling(), ++passIdx)
            {
                Heart::proto::MaterialPass* pass=findOrAddMaterialPass(tech, passIdx);

                if (xPass.FirstChild("blendenable").ToNode()) {
                    pass->mutable_blend()->set_blendenable(xPass.FirstChild("blendenable").GetValueEnum(g_trueFalseEnum, RSV_DISABLE));
                }
                if (xPass.FirstChild("blendop").ToNode()) {
                    pass->mutable_blend()->set_blendop(xPass.FirstChild("blendop").GetValueEnum(g_blendFuncEnum, RSV_BLEND_FUNC_ADD));
                }
                if (xPass.FirstChild("blendopalpha").ToNode()) {
                    pass->mutable_blend()->set_blendopalpha(xPass.FirstChild("blendopalpha").GetValueEnum(g_blendFuncEnum, RSV_BLEND_FUNC_ADD));
                }
                if (xPass.FirstChild("destblend").ToNode()) {
                    pass->mutable_blend()->set_destblend(xPass.FirstChild("destblend").GetValueEnum(g_blendOpEnum, RSV_BLEND_OP_ONE));
                }
                if (xPass.FirstChild("destblendalpha").ToNode()) {
                    pass->mutable_blend()->set_destblendalpha(xPass.FirstChild("destblendalpha").GetValueEnum(g_blendOpEnum, RSV_BLEND_OP_ONE));
                }
                if (xPass.FirstChild("srcblend").ToNode()) {
                    pass->mutable_blend()->set_srcblend(xPass.FirstChild("srcblend").GetValueEnum(g_blendOpEnum, RSV_BLEND_OP_ONE));
                }
                if (xPass.FirstChild("srcblendalpha").ToNode()) {
                    pass->mutable_blend()->set_srcblendalpha(xPass.FirstChild("srcblendalpha").GetValueEnum(g_blendOpEnum, RSV_BLEND_OP_ONE));
                }
                if (xPass.FirstChild("rendertargetwritemask").ToNode()) {
                    pass->mutable_blend()->set_rendertargetwritemask(xPass.FirstChild("rendertargetwritemask").GetValueHex(0xFF));
                }
                
                if (xPass.FirstChild("depthtest").ToNode()) {
                    pass->mutable_depthstencil()->set_depthenable(xPass.FirstChild("depthtest").GetValueEnum(g_trueFalseEnum, RSV_DISABLE));
                }
                if (xPass.FirstChild("depthfunc").ToNode()) {
                    pass->mutable_depthstencil()->set_depthfunc(xPass.FirstChild("depthfunc").GetValueEnum(g_depthEnum, RSV_Z_CMP_LESS));
                }
                if (xPass.FirstChild("depthwrite").ToNode()) {
                    pass->mutable_depthstencil()->set_depthwritemask(xPass.FirstChild("depthwrite").GetValueEnum(g_trueFalseEnum, RSV_DISABLE));
                }
                if (xPass.FirstChild("stencilenable").ToNode()) {
                    pass->mutable_depthstencil()->set_stencilenable(xPass.FirstChild("stencilenable").GetValueEnum(g_trueFalseEnum, RSV_DISABLE));
                }
                if (xPass.FirstChild("stencilfunc").ToNode()) {
                    pass->mutable_depthstencil()->set_stencilfunc(xPass.FirstChild("stencilfunc").GetValueEnum(g_stencilFuncEnum, RSV_SF_CMP_NEVER));
                }
                if (xPass.FirstChild("stencildepthfailop").ToNode()) {
                    pass->mutable_depthstencil()->set_stencildepthfailop(xPass.FirstChild("stencildepthfailop").GetValueEnum(g_stencilOpEnum, RSV_SO_KEEP));
                }
                if (xPass.FirstChild("stencilfail").ToNode()) {
                    pass->mutable_depthstencil()->set_stencilfailop(xPass.FirstChild("stencilfail").GetValueEnum(g_stencilOpEnum, RSV_SO_KEEP));
                }
                if (xPass.FirstChild("stencilpass").ToNode()) {
                    pass->mutable_depthstencil()->set_stencilpassop(xPass.FirstChild("stencilpass").GetValueEnum(g_stencilOpEnum, RSV_SO_KEEP));
                }
                if (xPass.FirstChild("stencilreadmask").ToNode()) {
                    pass->mutable_depthstencil()->set_stencilreadmask(xPass.FirstChild("stencilreadmask").GetValueHex(0xFFFFFFFF));
                }
                if (xPass.FirstChild("stencilwritemask").ToNode()) {
                    pass->mutable_depthstencil()->set_stencilwritemask(xPass.FirstChild("stencilwritemask").GetValueHex(0xFFFFFFFF));
                }
                if (xPass.FirstChild("stencilref").ToNode()) {
                    pass->mutable_depthstencil()->set_stencilref(xPass.FirstChild("stencilref").GetValueHex(0x00000000));
                }
                
                if (xPass.FirstChild("cullmode").ToNode()) {
                    pass->mutable_rasterizer()->set_cullmode(xPass.FirstChild("cullmode").GetValueEnum(g_cullModeEnum, RSV_CULL_MODE_CCW));
                }
                //passDef.rasterizerState.depthBias_ = 0;//xPass.FirstChild("depthbias").GetValueFloat();
                if (xPass.FirstChild("depthbiasclamp").ToNode()) {
                    pass->mutable_rasterizer()->set_depthbiasclamp(xPass.FirstChild("depthbiasclamp").GetValueFloat());
                }
                if (xPass.FirstChild("depthclipenable").ToNode()) {
                    pass->mutable_rasterizer()->set_depthclipenable(xPass.FirstChild("depthclipenable").GetValueEnum(g_trueFalseEnum, RSV_DISABLE));
                }
                if (xPass.FirstChild("fillmode").ToNode()) {
                    pass->mutable_rasterizer()->set_fillmode(xPass.FirstChild("fillmode").GetValueEnum(g_fillModeEnum, RSV_FILL_MODE_SOLID));
                }
                if (xPass.FirstChild("slopescaleddepthbias").ToNode()) {
                    pass->mutable_rasterizer()->set_slopescaleddepthbias(xPass.FirstChild("slopescaleddepthbias").GetValueFloat());
                }
                if (xPass.FirstChild("scissortest").ToNode()) {
                    pass->mutable_rasterizer()->set_scissorenable(xPass.FirstChild("scissortest").GetValueEnum(g_trueFalseEnum, RSV_DISABLE));
                }
                //passDef.rasterizerState.frontCounterClockwise_ = RSV_ENABLE;
                
                if (xPass.FirstChild("vertex").ToNode()) {
                    pass->set_vertex(xPass.FirstChild("vertex").GetValueString(""));
                    if (!pass->vertex().empty()) {
                        deps->push_back(xPass.FirstChild("vertex").GetValueString());
                    }
                }
                if (xPass.FirstChild("fragment").ToNode()) {
                    pass->set_pixel(xPass.FirstChild("fragment").GetValueString(""));
                    if (!pass->pixel().empty()) {
                        deps->push_back(xPass.FirstChild("fragment").GetValueString());
                    }
                }
                if (xPass.FirstChild("geometry").ToNode()) {
                    pass->set_geometry(xPass.FirstChild("geometry").GetValueString(""));
                    if (!pass->geometry().empty()) {
                        deps->push_back(xPass.FirstChild("geometry").GetValueString());
                    }
                }
                if (xPass.FirstChild("hull").ToNode()) {
                    pass->set_hull(xPass.FirstChild("hull").GetValueString(""));
                    if (!pass->hull().empty()) {
                        deps->push_back(xPass.FirstChild("hull").GetValueString());
                    }
                }
                if (xPass.FirstChild("domain").ToNode()) {
                    pass->set_domain(xPass.FirstChild("domain").GetValueString(""));
                    if (!pass->domain().empty()) {
                        deps->push_back(xPass.FirstChild("domain").GetValueString());
                    }
                }
            }
        }
    }
}

extern "C" {
//Lua entry point calls
DLL_EXPORT int MB_API luaopen_material(lua_State *L) {
    static const luaL_Reg materiallib[] = {
        {"build"      , materialCompile},
        {NULL, NULL}
    };
    luaL_newlib(L, materiallib);
    return 1;
}
}

