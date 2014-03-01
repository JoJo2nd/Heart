/********************************************************************

    filename: 	fontloader.cpp	
    
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

#include "cryptoBase64.h"
#include <boost/smart_ptr.hpp>
#include <boost/filesystem.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include "rapidxml/rapidxml.hpp"
#include "Heart.h"

#if defined (mesh_builder_EXPORTS)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif

#define MB_API   __cdecl

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct LODInfo
{
    hFloat maxRange;
};

struct MaterialMap
{
    const hChar*        first;
    const hChar*        second;
    Heart::hResourceID  resID;
};

struct StreamInfo
{
    Heart::hInputLayoutDesc   desc;
    hUint                     count;
    const char*               encodedData;
    hUint                     encodedDataSize;
    boost::shared_array<char> decodedData;
    hUint                     decodedDataSize;
};

//////////////////////////////////////////////////////////////////////////
// Enum Tables ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Heart::hXMLEnumReamp g_formatTypes[] =
{
    {"FLOAT1",      Heart::eIF_FLOAT1       },
    {"FLOAT2",      Heart::eIF_FLOAT2       },
    {"FLOAT3",      Heart::eIF_FLOAT3       },
    {"FLOAT4",      Heart::eIF_FLOAT4       },
    {"UBTYE4UNORM", Heart::eIF_UBYTE4_UNORM },
    {"UBTYE4SNORM", Heart::eIF_UBYTE4_SNORM },
};

Heart::hXMLEnumReamp g_semanticTypes[] =
{
    {"POSITION",    Heart::eIS_POSITION  },
    {"NORMAL",      Heart::eIS_NORMAL    },
    {"TEXCOORD",    Heart::eIS_TEXCOORD  },
    {"COLOUR",      Heart::eIS_COLOUR    },
    {"TANGENT",     Heart::eIS_TANGENT   },
    {"BITANGENT",   Heart::eIS_BITANGENT },
    {"INSTANCE",    Heart::eIS_INSTANCE  },
};
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void WriteLODRenderables(const Heart::hXMLGetter& xLODData, Heart::proto::Mesh* mesh, std::list< std::string >* depres);
void GetMeshBounds(const hFloat* in, hUint inele, hUint verts, hFloat* min, hFloat* max);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int MB_API meshCompile(lua_State* L)
{
    using namespace Heart;
    using namespace boost;
    /* Args from Lua (1: input files table, 2: dep files table, 3: parameter table, 4: outputpath)*/
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TTABLE);
    luaL_checktype(L, 3, LUA_TTABLE);
    luaL_checktype(L, 4, LUA_TSTRING);

    std::vector<std::string> openedfiles;

    lua_rawgeti(L, 1, 1);
    if (!lua_isstring(L, -1)) {
        luaL_error(L, "input file is not a string");
        return 0;
    }
    lua_getglobal(L, "buildpathresolve");
    lua_pushvalue(L, -2);
    lua_call(L, 1, 1);

    std::string filepath=lua_tostring(L, -1);
    hUint filesize;
    MeshHeader header = {0};
    system::error_code ec;
    rapidxml::xml_document<> xmldoc;
    shared_array<hChar> xmlmem;
    shared_array<LODInfo> lodInfo;
    std::list< std::string > dependentres;
    hChar* pathroot = (hChar*)hAlloca(strlen(filepath.c_str()));

    hChar* end = strrchr(pathroot, '/');
    if (end == NULL) 
        pathroot[0] = 0;
    else 
        end = NULL;
    filesize=(hUint)filesystem::file_size(filepath.c_str(), ec);
    if (ec) {
        luaL_error(L, "Failed to read % file size", filepath.c_str());
        return 0;
    }
    std::ifstream infile;

    infile.open(filepath);
    if (!infile.is_open()) {
        luaL_error(L, "Couldn't open file %s", filepath);
        return 0;
    }

    openedfiles.push_back(filepath);

    xmlmem = shared_array<hChar>(new hChar[filesize+1]);
    infile.read(xmlmem.get(), filesize);
    xmlmem[filesize] = 0;
    infile.close();

    try {
        xmldoc.parse< rapidxml::parse_default >(xmlmem.get());
    } catch (...) {
        luaL_error(L, "XML parse failed");
        return 0;
    }

    lua_getglobal(L, "buildpathresolve");
    lua_pushvalue(L, 4);
    lua_call(L, 1, 1);
    const hChar* outputpath=lua_tostring(L, -1);
    std::ofstream outfile;
    outfile.open(outputpath, std::ios_base::out|std::ios_base::binary);
    if (!outfile.is_open()) {
        luaL_error(L, "Failed to open output file %s", outputpath);
        return 0;
    }

    Heart::proto::Mesh meshresource;

    hXMLGetter xLODData = hXMLGetter(&xmldoc).FirstChild("modeldescription").FirstChild("lod");
    // only parse the first model
    if (xLODData.ToNode()) {
        WriteLODRenderables(xLODData, &meshresource, &dependentres);
    } else {
        luaL_error(L, "Mesh description is missing any mesh data");
    }

    std::string streambuffer;
    google::protobuf::io::StringOutputStream filestream(&streambuffer);
    google::protobuf::io::CodedOutputStream outputstream(&filestream);

    Heart::proto::ResourceHeader resHeader;
    resHeader.set_type("mesh");
    resHeader.set_sourcefile(lua_tostring(L, 4));
    Heart::proto::ResourceSection* blobsection = resHeader.add_sections();
    blobsection->set_type(Heart::proto::eResourceSection_Temp);
    blobsection->set_sectionname("mesh");
    blobsection->set_size(meshresource.ByteSize());

    Heart::serialiseToStreamWithSizeHeader(resHeader, &outputstream);
    meshresource.SerializeToCodedStream(&outputstream);
    outfile.write(streambuffer.c_str(), streambuffer.length());

#if 0
    //write the resource header
    hUint totalmeshsize=0;
    totalmeshsize+=sizeof(header);
    Heart::proto::ResourceHeader resHeader;
    resHeader.set_type("mesh");
    resHeader.set_sourcefile(lua_tostring(L, 4));
    Heart::proto::ResourceSection* blobsection = resHeader.add_sections();
    blobsection->set_type(Heart::proto::eResourceSection_Temp);
    blobsection->set_sectionname("mesh_blob");
    blobsection->set_size(totalmeshsize);

    Heart::serialiseToStreamWithSizeHeader(resHeader, &outputstream);
    outfile.write(streambuffer.c_str(), streambuffer.length());

    header.resHeader.resourceType = MESH_MAGIC_NUM;
    header.version = MESH_VERSION;
    header.lodCount=0;

    hXMLGetter xLODGetter = hXMLGetter(&xmldoc).FirstChild("modeldescription").FirstChild("lod");
    for (hUint32 i = 0;xLODGetter.ToNode(); xLODGetter = xLODGetter.NextSibling(), ++i) ++header.lodCount;
    lodInfo = shared_array<LODInfo>(new LODInfo[header.lodCount]);
    xLODGetter = hXMLGetter(&xmldoc).FirstChild("modeldescription").FirstChild("lod");
    for (hUint32 i = 0;xLODGetter.ToNode(); xLODGetter = xLODGetter.NextSibling(), ++i)
    {
        lodInfo[i].maxRange = xLODGetter.GetAttributeFloat("range",1000.f);
    }

    outfile.write((char*)&header, sizeof(header));

    lodIdx = 0;
    hXMLGetter xLODData = hXMLGetter(&xmldoc).FirstChild("modeldescription").FirstChild("lod");
    for (; xLODData.ToNode(); xLODData = xLODData.NextSibling(), ++lodIdx )
    {
        hUint64 writeOffset = outfile.tellp();
        LODHeader lodHeader = {0};
        lodHeader.minRange = lodIdx == 0 ? 0.0f : lodInfo[lodIdx-1].maxRange;
        lodHeader.maxRange = lodInfo[lodIdx].maxRange;

        // Write a dummy header, WriteLODRenderables will fill the correct data
        // for the header and we'll write it again
        outfile.write((char*)&lodHeader, sizeof(lodHeader));
        
        for (hUint32 i = 0; i < 3; ++i)
        {
            lodHeader.boundsMax[i] =  FLT_MAX;
            lodHeader.boundsMin[i] = -FLT_MAX;
        }

        WriteLODRenderables(xLODData, &lodHeader, &outfile, &dependentres);

        // Write out with the correct data
        outfile.seekp(writeOffset);
        outfile.write((char*)&lodHeader, sizeof(lodHeader));

        outfile.seekp(0, std::ios_base::end);
    }

    //re write the header
    auto writtensize=outfile.tellp();
    totalmeshsize=(hUint)((size_t)writtensize-(size_t)resHeader.ByteSize());
    blobsection->set_size(totalmeshsize);
    outfile.seekp(0, std::ios_base::beg);
    Heart::serialiseToStreamWithSizeHeader(resHeader, &outputstream);
#endif

    //Return a list of resources this material is dependent on
    dependentres.unique();
    lua_newtable(L);
    hUint idx=1;
    for (auto i=dependentres.begin(),n=dependentres.end(); i!=n; ++i) {
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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void WriteLODRenderables(const Heart::hXMLGetter& xLODData, Heart::proto::Mesh* mesh, std::list< std::string >* depres) {
    using namespace boost;

    Heart::hXMLGetter renderablenode = xLODData.FirstChild("renderable");
    for (; renderablenode.ToNode(); renderablenode=renderablenode.NextSibling()) {
        Heart::proto::Renderable* renderablebuf=mesh->add_renderables();
        Heart::hXMLGetter indexnode=renderablenode.FirstChild("index");
        hUint vtxcount=0;
        hFloat boundsMin[3];
        hFloat boundsMax[3];
        for (Heart::hXMLGetter s=renderablenode.FirstChild("stream"); s.ToNode(); s=s.NextSibling()) {
            Heart::proto::VertexStream* streambuf=renderablebuf->add_vertexstreams();
            hUint format = s.GetAttributeEnum("type", g_formatTypes, Heart::eIF_FLOAT1);
            hUint streamcount=(hUint32)s.GetAttributeInt("count", 0);
            vtxcount=hMax(streamcount, vtxcount);
            streambuf->set_format(format);
            streambuf->set_semantic(s.GetAttributeString("semantic"));
            streambuf->set_semanticindex(s.GetAttributeInt("index", 0));
            
            const hChar* encodeddata=s.GetValueString();
            hUint encodeddatasize=s.GetValueStringLen();
            hUint decodeddatasize=cyBase64DecodeCalcRequiredSize(encodeddata, encodeddatasize);
            auto decodeddata=shared_array<char>(new char[decodeddatasize]);
            cyBase64Decode(encodeddata, encodeddatasize, decodeddata.get(), decodeddatasize);

            if (Heart::hStrICmp(s.GetAttributeString("semantic"), "POSITION") == 0) {
                GetMeshBounds((hFloat*)decodeddata.get(), format, streamcount, boundsMin, boundsMax);
            }

            streambuf->set_streamdata(decodeddata.get(), decodeddatasize);
        }

        depres->push_back(renderablenode.GetAttributeString("material"));
        renderablebuf->set_vertexcount(vtxcount);
        renderablebuf->set_primtype(Heart::PRIMITIVETYPE_TRILIST);
        renderablebuf->set_materialresource(Heart::hResourceManager::BuildResourceID(renderablenode.GetAttributeString("material")));
        if (indexnode.ToNode()) {
            hUint indexcount=indexnode.GetAttributeInt("count",0);
            hUint ibsize=cyBase64DecodeCalcRequiredSize(indexnode.GetValueString(), indexnode.GetValueStringLen());
            scoped_ptr<char> indexData(new char[ibsize]);
            cyBase64Decode(indexnode.GetValueString(), indexnode.GetValueStringLen(), indexData.get(), ibsize);
            
            renderablebuf->set_indexcount(indexcount);
            renderablebuf->set_indexbuffer(indexData.get(), ibsize);
            renderablebuf->set_primcount(indexcount/3);
        } else {
            renderablebuf->set_primcount(vtxcount/3);
        }

        renderablebuf->mutable_aabb()->set_minx(boundsMin[0]);
        renderablebuf->mutable_aabb()->set_miny(boundsMin[1]);
        renderablebuf->mutable_aabb()->set_minz(boundsMin[2]);
        renderablebuf->mutable_aabb()->set_maxx(boundsMax[0]);
        renderablebuf->mutable_aabb()->set_maxy(boundsMax[1]);
        renderablebuf->mutable_aabb()->set_maxz(boundsMax[2]);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void GetMeshBounds(const hFloat* in, hUint inele, hUint verts, hFloat* min, hFloat* max)
{
    for (hUint i = 0; i < 3; ++i)
    {
        min[i] = FLT_MAX;
        max[i] = -FLT_MAX;
    }
    for (hUint t=0,n=verts*inele; t < n; t+=inele)  {
        for (hUint i=0; i<inele; ++i) {
            min[i]=hMin(min[i],in[t+i]);
            max[i]=hMax(max[i],in[t+i]);
        }
        for (hUint i=inele; i<3; ++i) {
            min[i]=-.5f;max[i]=.5f;
        }
    }
}

extern "C" {
//Lua entry point calls
DLL_EXPORT int MB_API luaopen_mesh(lua_State *L) {
    static const luaL_Reg meshlib[] = {
        {"compile"      , meshCompile},
        {NULL, NULL}
    };
    luaL_newlib(L, meshlib);
    return 1;
}
}

