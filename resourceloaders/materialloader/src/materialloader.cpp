/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "rapidxml/rapidxml.hpp"
#include "utils/hRapidXML.h"
#include "minfs.h"
#include "resource_material_fx.pb.h"
#include "builder.pb.h"
#include "getopt.h"

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

#include <fstream>
#include <vector>
#include <string>
#include <stdio.h>
#include <iostream>

#ifdef _WIN32
#   include <io.h>
#   include <fcntl.h>
#endif

static const char argopts[] = "vi:";
static struct option long_options[] = {
    { "version", no_argument, 0, 'z' },
    { 0, 0, 0, 0 }
};

#define fatal_error_check(x, msg, ...) if (!(x)) {fprintf(stderr, msg, __VA_ARGS__); exit(-1);}
#define fatal_error(msg, ...) fatal_error_check(false, msg, __VA_ARGS__)

typedef std::vector< std::string > StrVectorType;

//////////////////////////////////////////////////////////////////////////
// Enum Tables ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Heart::hXMLEnumReamp g_parameterTypes[] =
{
    {"float",   Heart::proto::matparam_float  },
    {"int",     Heart::proto::matparam_int    },
    {"texture", Heart::proto::matparam_texture},
    {"none",    Heart::proto::matparam_none   },
};


Heart::hXMLEnumReamp g_samplerStates[] =
{
    { "repeat",                  Heart::proto::renderstate::wrap },
    { "wrap",                    Heart::proto::renderstate::wrap },

    { "clamp",                   Heart::proto::renderstate::clamp },
    { "clamptoedge",             Heart::proto::renderstate::clamp },
    { "mirror",                  Heart::proto::renderstate::mirror },
    { "border",                  Heart::proto::renderstate::border },
    { "aniso",                   Heart::proto::renderstate::anisotropic },
    { "point",                   Heart::proto::renderstate::point },
    { "linear",                  Heart::proto::renderstate::linear },
    { "linearmipmaplinear",      Heart::proto::renderstate::linear },
    { NULL,                      0}
};

Heart::hXMLEnumReamp g_trueFalseEnum[] = 
{
    { "true", 1 },
    { "false", 0 },
    {NULL, 0}
};

Heart::hXMLEnumReamp g_blendFuncEnum[] = 
{
    { "funcadd" , Heart::proto::renderstate::Add },
    { "funcsubtract" , Heart::proto::renderstate::Sub },
    { "add" , Heart::proto::renderstate::Add },
    { "subtract" , Heart::proto::renderstate::Sub },
    { "min" , Heart::proto::renderstate::Min },
    { "max" , Heart::proto::renderstate::Max },
    {NULL, 0}
};
Heart::hXMLEnumReamp g_blendOpEnum[] =
{
    { "zero" , Heart::proto::renderstate::BlendZero },
    { "one" , Heart::proto::renderstate::BlendOne },
    { "destcolor" , Heart::proto::renderstate::BlendDestColour },
    { "invdestcolor" , Heart::proto::renderstate::BlendInverseDestColour },
    { "srcalpha" , Heart::proto::renderstate::BlendSrcAlpha }, 
    { "invsrcalpha" , Heart::proto::renderstate::BlendInverseSrcAlpha },
    { "dstalpha" , Heart::proto::renderstate::BlendDestAlpha },
    { "invdestalpha" , Heart::proto::renderstate::BlendInverseDestAlpha },
    { "srccolor" , Heart::proto::renderstate::BlendSrcColour },
    { "invsrccolor" , Heart::proto::renderstate::BlendInverseSrcColour },
    {NULL, 0}
};
Heart::hXMLEnumReamp g_depthEnum[] = 
{
    { "never" , Heart::proto::renderstate::CompareNever },
    { "less" , Heart::proto::renderstate::CompareLess },
    { "lequal" , Heart::proto::renderstate::CompareLessEqual },
    { "lessequal" , Heart::proto::renderstate::CompareLessEqual },
    { "equal" , Heart::proto::renderstate::CompareEqual },
    { "greater" , Heart::proto::renderstate::CompareGreater },
    { "notequal" , Heart::proto::renderstate::CompareNotEqual },
    { "gequal" , Heart::proto::renderstate::CompareGreaterEqual },
    { "greaterequal" , Heart::proto::renderstate::CompareGreater },
    { "always" , Heart::proto::renderstate::CompareAlways },
    {NULL, 0}
};
Heart::hXMLEnumReamp g_fillModeEnum[] = 
{
    { "solid" , Heart::proto::renderstate::Solid },
    { "wireframe" , Heart::proto::renderstate::Wireframe },
    {NULL, 0}
};
Heart::hXMLEnumReamp g_cullModeEnum[] =
{
    { "none" , Heart::proto::renderstate::CullNone },
    { "cw" , Heart::proto::renderstate::CullClockwise },
    { "ccw" , Heart::proto::renderstate::CullCounterClockwise },
    {NULL, 0}
};
Heart::hXMLEnumReamp g_stencilFuncEnum[] =
{
    { "never" , Heart::proto::renderstate::CompareNever },
    { "less" , Heart::proto::renderstate::CompareLess },
    { "lequal" , Heart::proto::renderstate::CompareLessEqual },
    { "lessequal" , Heart::proto::renderstate::CompareLessEqual },
    { "equal" , Heart::proto::renderstate::CompareEqual },
    { "greater" , Heart::proto::renderstate::CompareGreater },
    { "notequal" , Heart::proto::renderstate::CompareNotEqual },
    { "gequal" , Heart::proto::renderstate::CompareGreaterEqual },
    { "greaterequal" , Heart::proto::renderstate::CompareGreaterEqual },
    { "always" , Heart::proto::renderstate::CompareAlways },
    {NULL, 0}
};
Heart::hXMLEnumReamp g_stencilOpEnum[] = 
{
    { "keep" ,      Heart::proto::renderstate::StencilKeep, },
    { "zero" ,      Heart::proto::renderstate::StencilZero, },
    { "replace" ,   Heart::proto::renderstate::StencilReplace, },
    { "incr" ,      Heart::proto::renderstate::StencilIncr, },
    { "decr" ,      Heart::proto::renderstate::StencilDecr, },
    { "invert" ,    Heart::proto::renderstate::StencilInvert, },
    { "incrsat" ,   Heart::proto::renderstate::StencilIncSat },
    { "decrsat" ,   Heart::proto::renderstate::StencilDecSat },
    {NULL, 0}
};

#define luaL_errorthrow(L, fmt, ...) \
    luaL_where(L, 1); \
    lua_pushfstring(L, fmt, ##__VA_ARGS__); \
    lua_concat(L, 2); \
    throw std::exception();

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void readMaterialXMLToMaterialData(const rapidxml::xml_document<>& xmldoc, const hChar* xmlpath, Heart::proto::MaterialResource* mat, StrVectorType* includes, StrVectorType* deps);

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

Heart::proto::MaterialTechnique* findMaterialTechnique(Heart::proto::MaterialGroup* group, const hChar* name) {
    for (hUint i=0, n=group->technique_size(); i<n; ++i) {
        if (Heart::hStrCmp(group->technique(i).techniquename().c_str(), name) == 0) {
            return group->mutable_technique()->Mutable(i);
        }
    }
    return nullptr;
}

Heart::proto::MaterialTechnique* findOrAddMaterialTechnique(Heart::proto::MaterialGroup* group, const hChar* name) {
    auto r = findMaterialTechnique(group, name);
    if (r)
        return r;
    r = group->add_technique();
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

int main(int argc, char* argv[]) {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
#endif
    using namespace Heart;

    google::protobuf::io::IstreamInputStream input_stream(&std::cin);
    Heart::builder::Input input_pb;

    int c;
    int option_index = 0;
    bool verbose = false, use_stdin = true;

    while ((c = gop_getopt_long(argc, argv, argopts, long_options, &option_index)) != -1) {
        switch (c) {
        case 'z': fprintf(stdout, "heart material builder v0.8.0"); exit(0);
        case 'v': verbose = 1; break;
        case 'i': {
            std::ifstream input_file_stream;
            input_file_stream.open(optarg, std::ios_base::binary | std::ios_base::in);
            if (input_file_stream.is_open()) {
                google::protobuf::io::IstreamInputStream file_stream(&input_file_stream);
                input_pb.ParseFromZeroCopyStream(&file_stream);
                use_stdin = false;
            }
        } break;
        default: return 2;
        }
    }

    if (use_stdin) {
        input_pb.ParseFromZeroCopyStream(&input_stream);
    }

    /* Args from Lua (1: input files table, 2: dep files table, 3: parameter table, 4: outputpath)*/
    Heart::proto::MaterialResource materialresource;
    std::vector<std::string> openedfiles;
    std::string filepath;
    rapidxml::xml_document<> xmldoc;
    hUint filesize=0;
    StrVectorType depresnames;
    StrVectorType includes;

    filepath=input_pb.resourceinputpath();
    filesize=(hUint)minfs_get_file_size(filepath.c_str());

    std::vector<hChar> xmlmem(filesize+1);
    std::ifstream infile;
    infile.open(filepath);
    if (!infile.is_open()) {
        fatal_error("Unable to open file %s", filepath.c_str());
    }
    memset(xmlmem.data(), 0, filesize+1);
    infile.read(xmlmem.data(), filesize);
    infile.close();
    openedfiles.push_back(filepath);
    try {
        xmldoc.parse< rapidxml::parse_default >(xmlmem.data());
    } catch (...) {
        fatal_error("Failed to parse material file");
    }

    readMaterialXMLToMaterialData(xmldoc, filepath.c_str(), &materialresource, &includes, &depresnames);

    for (auto i=includes.begin(),n=includes.end(); i!=n; ++i) {
        openedfiles.push_back(*i);
    }

    Heart::builder::Output output;
    output.add_filedependency(filepath);
    //write the resource header
    output.mutable_pkgdata()->set_type_name(materialresource.GetTypeName());
    output.mutable_pkgdata()->set_messagedata(materialresource.SerializeAsString());

    //Return a list of resources this material is dependent on
    for (const auto& i : depresnames) {
        output.add_resourcedependency(i);
    }
    for (const auto& i : openedfiles) {
        output.add_filedependency(i);
    }

    google::protobuf::io::OstreamOutputStream filestream(&std::cout);
    return output.SerializeToZeroCopyStream(&filestream) ? 0 : -2;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void readMaterialXMLToMaterialData(const rapidxml::xml_document<>& xmldoc, const hChar* xmlpath, Heart::proto::MaterialResource* mat, StrVectorType* includes, StrVectorType* deps) {
    using namespace Heart;

    if (hXMLGetter(&xmldoc).FirstChild("material").GetAttributeString("inherit")) {
        //Load the base first
        const hChar* basepathrel=hXMLGetter(&xmldoc).FirstChild("material").GetAttributeString("inherit");
        hUint pathlen=(hUint)strlen(basepathrel)+(hUint)strlen(xmlpath)+1;//+1 for NULL
        hChar* fullbasepath=(hChar*)hAlloca(pathlen);
        hChar* pathSep=NULL;
        strncpy(fullbasepath, xmlpath, pathlen);
        pathSep=strrchr(fullbasepath, '/');
        if (pathSep) {
            *(pathSep+1)=0;
            strncat(fullbasepath, basepathrel, pathlen-strlen(xmlpath));
        } else {
            strncpy(fullbasepath, basepathrel, pathlen-strlen(xmlpath));
        }
        std::vector<hChar> ptr;
        std::ifstream incfile;
        hUint incfilesize=(hUint)minfs_get_file_size(fullbasepath);
        incfile.open(fullbasepath);
        if (incfile.is_open()) {
            rapidxml::xml_document<> xmldocbase;
            ptr.resize(incfilesize+1);
            memset(ptr.data(), 0, incfilesize);
            incfile.read(ptr.data(), incfilesize);
            try {
                xmldocbase.parse< rapidxml::parse_default >(ptr.data());
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
        mat->set_totalsamplers(mat->totalsamplers()+1);
        Heart::proto::MaterialSampler* sampler=nullptr;
        if (xSampler.GetAttribute("name")) {
            sampler = findOrAddMaterialSampler(mat, xSampler.GetAttribute("name")->value());
        } else { // Error? can't process this!
            fatal_error("Can't find name parameter for sampler in material");
        }

        if (xSampler.FirstChild("addressu").ToNode()) {
            sampler->mutable_samplerstate()->set_addressu(xSampler.FirstChild("addressu").GetValueEnum(g_samplerStates, Heart::proto::renderstate::clamp));
        }
        if (xSampler.FirstChild("addressv").ToNode()) {
            sampler->mutable_samplerstate()->set_addressv(xSampler.FirstChild("addressv").GetValueEnum(g_samplerStates, Heart::proto::renderstate::clamp));
        }
        if (xSampler.FirstChild("addressw").ToNode()) {
            sampler->mutable_samplerstate()->set_addressw(xSampler.FirstChild("addressw").GetValueEnum(g_samplerStates, Heart::proto::renderstate::clamp));
        }
        if (xSampler.FirstChild("bordercolour").ToNode()) {
            auto colour=xSampler.FirstChild("bordercolour").GetValueColour(hColour(0.f, 0.f, 0.f, 1.f));
            sampler->mutable_samplerstate()->mutable_bordercolour()->set_red((hUint)(colour.r_*255.f+.5f));
            sampler->mutable_samplerstate()->mutable_bordercolour()->set_green((hUint)(colour.g_*255.f+.5f));
            sampler->mutable_samplerstate()->mutable_bordercolour()->set_blue((hUint)(colour.b_*255.f+.5f));
            sampler->mutable_samplerstate()->mutable_bordercolour()->set_alpha((hUint)(colour.a_*255.f+.5f));
        }
        if (xSampler.FirstChild("filter").ToNode()) {
            sampler->mutable_samplerstate()->set_filter(xSampler.FirstChild("filter").GetValueEnum(g_samplerStates, Heart::proto::renderstate::point));
        }
        if (xSampler.FirstChild("maxanisotropy").ToNode()) {
            sampler->mutable_samplerstate()->set_maxanisotropy(xSampler.FirstChild("maxanisotropy").GetValueInt(1));
        }
        if (xSampler.FirstChild("minlod").ToNode()) {
            sampler->mutable_samplerstate()->set_minlod(xSampler.FirstChild("minlod").GetValueFloat());
        }
        if (xSampler.FirstChild("maxlod").ToNode()) {
            sampler->mutable_samplerstate()->set_maxlod(xSampler.FirstChild("maxlod").GetValueFloat(FLT_MAX));
        }
        if (xSampler.FirstChild("miplodbias").ToNode()) {
            sampler->mutable_samplerstate()->set_miplodbias(xSampler.FirstChild("miplodbias").GetValueFloat());
        }
    }

    hXMLGetter xParameter = hXMLGetter(&xmldoc).FirstChild("material").FirstChild("parameter");
    for (; xParameter.ToNode(); xParameter = xParameter.NextSibling()) {
        proto::MaterialParameterType ptype=proto::matparam_none;
        proto::MaterialParameter* param=nullptr;
        mat->set_totalparameters(mat->totalparameters()+1);
        if (xParameter.GetAttribute("name")) {
            param=findOrAddMaterialParameter(mat, xParameter.GetAttributeString("name","none"));
        } else {
            fatal_error("Can't find name for parameter in material");
        }
        if (xParameter.GetAttribute("type")) {
            ptype = xParameter.GetAttributeEnum("type", g_parameterTypes, proto::matparam_none );
        }
        if (xParameter.GetValueString()) {
            const hChar* valstr=xParameter.GetValueString();
            if (ptype == Heart::proto::matparam_float) {
                hFloat fd[16];
                int count=sscanf(valstr, " %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f ",
                    fd, fd+1, fd+2, fd+3, fd+4, fd+5, fd+6, fd+7, fd+8, fd+9, fd+10, fd+11, fd+12, fd+13, fd+14, fd+15);
                for (int i=0; i<count; ++i) {
                    param->add_floatvalues(fd[i]);
                }
            } else if (ptype == Heart::proto::matparam_int) {
                hInt id[16];
                int count=sscanf(valstr, " %d , %d , %d , %d , %d , %d , %d , %d , %d , %d , %d , %d , %d , %d , %d , %d ",
                    id, id+1, id+2, id+3, id+4, id+5, id+6, id+7, id+8, id+9, id+10, id+11, id+12, id+13, id+14, id+15);
                for (int i=0; i<count; ++i) {
                    param->add_intvalues(id[i]);
                }
            } else if (ptype == Heart::proto::matparam_texture) {
                param->set_resourceid(valstr);
            }
        }
    }

    hXMLGetter xGroup = hXMLGetter(&xmldoc).FirstChild("material").FirstChild("group");
    for (; xGroup.ToNode(); xGroup = xGroup.NextSibling())
    {
        Heart::proto::MaterialGroup* group=nullptr;
        if (xGroup.GetAttribute("name")) {
            group=findOrAddMaterialGroup(mat, xGroup.GetAttributeString("name","none"));
        } else {
            fatal_error("Material group is missing name");
        }
        mat->set_totalgroups(mat->totalgroups()+1);
        hXMLGetter xTech = xGroup.FirstChild("technique");
        for (; xTech.ToNode(); xTech = xTech.NextSibling())
        {
            Heart::proto::MaterialTechnique* tech=nullptr;
            if (!findMaterialTechnique(group, xTech.GetAttributeString("name","none"))) {
                mat->set_totaltechniques(mat->totaltechniques()+1);
            }

            if (xTech.GetAttribute("name")) {
                tech=findOrAddMaterialTechnique(group, xTech.GetAttributeString("name","none"));
            } else {
                fatal_error("Material technique is missing name");
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
                mat->set_totalpasses(mat->totalpasses()+1);
                if (xPass.FirstChild("blendenable").ToNode()) {
                    pass->mutable_blend()->set_blendenable(xPass.FirstChild("blendenable").GetValueEnum(g_trueFalseEnum, false));
                }
                if (xPass.FirstChild("blendop").ToNode()) {
                    pass->mutable_blend()->set_blendop(xPass.FirstChild("blendop").GetValueEnum(g_blendFuncEnum, Heart::proto::renderstate::Add));
                }
                if (xPass.FirstChild("blendopalpha").ToNode()) {
                    pass->mutable_blend()->set_blendopalpha(xPass.FirstChild("blendopalpha").GetValueEnum(g_blendFuncEnum, Heart::proto::renderstate::Add));
                }
                if (xPass.FirstChild("destblend").ToNode()) {
                    pass->mutable_blend()->set_destblend(xPass.FirstChild("destblend").GetValueEnum(g_blendOpEnum, Heart::proto::renderstate::BlendOne));
                }
                if (xPass.FirstChild("destblendalpha").ToNode()) {
                    pass->mutable_blend()->set_destblendalpha(xPass.FirstChild("destblendalpha").GetValueEnum(g_blendOpEnum, Heart::proto::renderstate::BlendOne));
                }
                if (xPass.FirstChild("srcblend").ToNode()) {
                    pass->mutable_blend()->set_srcblend(xPass.FirstChild("srcblend").GetValueEnum(g_blendOpEnum, Heart::proto::renderstate::BlendOne));
                }
                if (xPass.FirstChild("srcblendalpha").ToNode()) {
                    pass->mutable_blend()->set_srcblendalpha(xPass.FirstChild("srcblendalpha").GetValueEnum(g_blendOpEnum, Heart::proto::renderstate::BlendOne));
                }
                if (xPass.FirstChild("rendertargetwritemask").ToNode()) {
                    pass->mutable_blend()->set_rendertargetwritemask(xPass.FirstChild("rendertargetwritemask").GetValueHex(0xFF));
                }
                
                if (xPass.FirstChild("depthtest").ToNode()) {
                    pass->mutable_depthstencil()->set_depthenable(xPass.FirstChild("depthtest").GetValueEnum(g_trueFalseEnum, false));
                }
                if (xPass.FirstChild("depthfunc").ToNode()) {
                    pass->mutable_depthstencil()->set_depthfunc(xPass.FirstChild("depthfunc").GetValueEnum(g_depthEnum, Heart::proto::renderstate::CompareLess));
                }
                if (xPass.FirstChild("depthwrite").ToNode()) {
                    pass->mutable_depthstencil()->set_depthwritemask(xPass.FirstChild("depthwrite").GetValueEnum(g_trueFalseEnum, false));
                }
                if (xPass.FirstChild("stencilenable").ToNode()) {
                    pass->mutable_depthstencil()->set_stencilenable(xPass.FirstChild("stencilenable").GetValueEnum(g_trueFalseEnum, false));
                }
                if (xPass.FirstChild("stencilfunc").ToNode()) {
                    pass->mutable_depthstencil()->set_stencilfunc(xPass.FirstChild("stencilfunc").GetValueEnum(g_stencilFuncEnum, Heart::proto::renderstate::CompareNever));
                }
                if (xPass.FirstChild("stencildepthfailop").ToNode()) {
                    pass->mutable_depthstencil()->set_stencildepthfailop(xPass.FirstChild("stencildepthfailop").GetValueEnum(g_stencilOpEnum, Heart::proto::renderstate::StencilKeep));
                }
                if (xPass.FirstChild("stencilfail").ToNode()) {
                    pass->mutable_depthstencil()->set_stencilfailop(xPass.FirstChild("stencilfail").GetValueEnum(g_stencilOpEnum, Heart::proto::renderstate::StencilKeep));
                }
                if (xPass.FirstChild("stencilpass").ToNode()) {
                    pass->mutable_depthstencil()->set_stencilpassop(xPass.FirstChild("stencilpass").GetValueEnum(g_stencilOpEnum, Heart::proto::renderstate::StencilKeep));
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
                    pass->mutable_rasterizer()->set_cullmode(xPass.FirstChild("cullmode").GetValueEnum(g_cullModeEnum, Heart::proto::renderstate::CullCounterClockwise));
                }
                //passDef.rasterizerState.depthBias_ = 0;//xPass.FirstChild("depthbias").GetValueFloat();
                if (xPass.FirstChild("depthbiasclamp").ToNode()) {
                    pass->mutable_rasterizer()->set_depthbiasclamp(xPass.FirstChild("depthbiasclamp").GetValueFloat());
                }
                if (xPass.FirstChild("depthclipenable").ToNode()) {
                    pass->mutable_rasterizer()->set_depthclipenable(xPass.FirstChild("depthclipenable").GetValueEnum(g_trueFalseEnum, false));
                }
                if (xPass.FirstChild("fillmode").ToNode()) {
                    pass->mutable_rasterizer()->set_fillmode(xPass.FirstChild("fillmode").GetValueEnum(g_fillModeEnum, Heart::proto::renderstate::Solid));
                }
                if (xPass.FirstChild("slopescaleddepthbias").ToNode()) {
                    pass->mutable_rasterizer()->set_slopescaleddepthbias(xPass.FirstChild("slopescaleddepthbias").GetValueFloat());
                }
                if (xPass.FirstChild("scissortest").ToNode()) {
                    pass->mutable_rasterizer()->set_scissorenable(xPass.FirstChild("scissortest").GetValueEnum(g_trueFalseEnum, false));
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

