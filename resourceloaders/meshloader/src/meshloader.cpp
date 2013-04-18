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

#include "meshloader.h"
#include "cryptoBase64.h"
#include <boost/smart_ptr.hpp>
#include <boost/filesystem.hpp>

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

void WriteLODRenderables(const Heart::hXMLGetter& xLODData, LODHeader* header, std::ofstream* binoutput, std::list< std::string >* depres);
void GetMeshBounds(const hFloat* in, hUint inele, hUint verts, hFloat* min, hFloat* max);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int inputDescQsortCompar(const void* a, const void* b) {
    StreamInfo* lhs=(StreamInfo*)a;
    StreamInfo* rhs=(StreamInfo*)b;
    return ((0xFF-(lhs->desc.inputStream_-rhs->desc.inputStream_)) << 16) | (0xFF-(lhs->desc.semantic_-rhs->desc.semantic_));
}

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

    lua_rawgeti(L, 1, 1);
    if (!lua_isstring(L, -1)) {
        luaL_error(L, "input file is not a string");
        return 0;
    }
    lua_getglobal(L, "buildpathresolve");
    lua_pushvalue(L, -2);
    lua_call(L, 1, 1);
    const hChar* filepath=lua_tostring(L, -1);
    hUint filesize;
    MeshHeader header = {0};
    system::error_code ec;
    rapidxml::xml_document<> xmldoc;
    shared_array<hChar> xmlmem;
    shared_array<LODInfo> lodInfo;
    hUint32 lodIdx;
    std::list< std::string > dependentres;
    hChar* pathroot = (hChar*)hAlloca(strlen(filepath));

    hChar* end = strrchr(pathroot, '/');
    if (end == NULL) 
        pathroot[0] = 0;
    else 
        end = NULL;
    filesize=(hUint)filesystem::file_size(filepath, ec);
    if (ec) {
        luaL_error(L, "Failed to read % file size", filepath);
        return 0;
    }
    std::ifstream infile;

    infile.open(filepath);
    if (!infile.is_open()) {
        luaL_error(L, "Couldn't open file %s", filepath);
        return 0;
    }

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
    outfile.open(outputpath);
    if (!outfile.is_open()) {
        luaL_error(L, "Failed to open output file %s", outputpath);
        return 0;
    }

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

    //Return a list of resources this material is dependent on
    dependentres.unique();
    lua_newtable(L);
    hUint idx=1;
    for (std::list< std::string >::iterator i=dependentres.begin(),n=dependentres.end(); i!=n; ++i) {
        lua_pushstring(L, i->c_str());
        lua_rawseti(L, -2, idx);
        ++idx;
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void WriteLODRenderables(const Heart::hXMLGetter& xLODData, LODHeader* header, std::ofstream* binoutput, std::list< std::string >* depres) {
    using namespace boost;

    Heart::hXMLGetter renderablenode = xLODData.FirstChild("renderable");
    header->renderableCount = 0;
    header->renderableOffset = binoutput->tellp();

    for (; renderablenode.ToNode(); renderablenode=renderablenode.NextSibling()) {
        RenderableHeader renderableHeader = {0};
        hUint64 writeOffset = binoutput->tellp();
        Heart::hXMLGetter indexnode=renderablenode.FirstChild("index");
        hUint streamcount=0;
        for (Heart::hXMLGetter s=renderablenode.FirstChild("stream"); s.ToNode(); s=s.NextSibling()) ++streamcount;
        StreamInfo posStream;
        scoped_array<Heart::hXMLGetter> streamnodes(new Heart::hXMLGetter[streamcount]);
        scoped_array<StreamInfo> streaminfos(new StreamInfo[streamcount]);
        hUint streamidx=0;
        for (Heart::hXMLGetter s=renderablenode.FirstChild("stream"); s.ToNode(); s=s.NextSibling(), ++streamidx) {
            streamnodes[streamidx]=s;
            streaminfos[streamidx].encodedData=s.GetValueString();
            streaminfos[streamidx].encodedDataSize=s.GetValueStringLen();
            streaminfos[streamidx].decodedDataSize=cyBase64DecodeCalcRequiredSize(streaminfos[streamidx].encodedData, streaminfos[streamidx].encodedDataSize);
            streaminfos[streamidx].decodedData= shared_array<char>(new char[streaminfos[streamidx].decodedDataSize]);
            streaminfos[streamidx].count=s.GetAttributeInt("count", 0);
            streaminfos[streamidx].desc.inputStream_=s.GetAttributeInt("sindex", 0);
            streaminfos[streamidx].desc.semIndex_=(hByte)s.GetAttributeInt("index", 0);
            streaminfos[streamidx].desc.semantic_=s.GetAttributeEnum("semantic", g_semanticTypes, Heart::eIS_POSITION);
            streaminfos[streamidx].desc.typeFormat_=s.GetAttributeEnum("type", g_formatTypes, Heart::eIF_FLOAT1);
            streaminfos[streamidx].desc.instanceDataRepeat_=0;
            cyBase64Decode(streaminfos[streamidx].encodedData, streaminfos[streamidx].encodedDataSize, 
                streaminfos[streamidx].decodedData.get(), streaminfos[streamidx].decodedDataSize);
            if (streaminfos[streamidx].desc.semantic_==Heart::eIS_POSITION) {
                posStream=streaminfos[streamidx];
            }
        }
        ++header->renderableCount;

        renderableHeader.flags=0;
        renderableHeader.primType = Heart::PRIMITIVETYPE_TRILIST;
        renderableHeader.startIndex = 0;
        renderableHeader.materialID = Heart::hResourceManager::BuildResourceID(renderablenode.GetAttributeString("material"));
        depres->push_back(renderablenode.GetAttributeString("material"));
        if (indexnode.ToNode()) {
            renderableHeader.nPrimatives = indexnode.GetAttributeInt("count",0);
            renderableHeader.nPrimatives /= 3;
            renderableHeader.flags|=indexnode.GetAttributeInt("count",0) > 0xFFFF ? MESH_DATA_FLAG_32BIT_INDEX : 0;
        } else {
            renderableHeader.nPrimatives = posStream.count/3;
        }

        // Update bounds
        GetMeshBounds((hFloat*)posStream.decodedData.get(), (posStream.desc.typeFormat_-Heart::eIF_FLOAT1)+1, 
            posStream.count, renderableHeader.boundsMin, renderableHeader.boundsMax);
        for (hUint32 i = 0; i < 3; ++i)
        {
            header->boundsMin[i] = hMin(header->boundsMin[i], renderableHeader.boundsMin[i]);
            header->boundsMax[i] = hMax(header->boundsMax[i], renderableHeader.boundsMax[i]);
        }

        hUint vertexStreamCount=0;
        scoped_array<StreamInfo> inputDesc(new StreamInfo[streamcount]);
        Heart::hMemCpy(inputDesc.get(), streaminfos.get(), sizeof(StreamInfo)*streamcount);
        qsort(inputDesc.get(), streamcount, sizeof(StreamInfo), inputDescQsortCompar); //Sort by vertex stream index
        vertexStreamCount=inputDesc[streamcount-1].desc.inputStream_+1;
        binoutput->write((char*)&renderableHeader, sizeof(renderableHeader));
        for (hUint i=0; i<streamcount; ++i) {
            binoutput->write((char*)&inputDesc[i].desc, sizeof(Heart::hInputLayoutDesc));
        }
        renderableHeader.ibOffset = binoutput->tellp();
        if (indexnode.ToNode()) {
            hUint ibsize=cyBase64DecodeCalcRequiredSize(indexnode.GetValueString(), indexnode.GetValueStringLen());
            scoped_ptr<char> indexData(new char[ibsize]);
            cyBase64Decode(indexnode.GetValueString(), indexnode.GetValueStringLen(), indexData.get(), ibsize);
            binoutput->write(indexData.get(), ibsize);
            renderableHeader.ibSize=ibsize;
            //hcAssert(renderableHeader.nPrimatives*3*sizeof(hUint32) == ibsize || renderableHeader.nPrimatives*3*sizeof(hUint16) == ibsize);
        } else {
            renderableHeader.ibSize=0;
        }

        renderableHeader.verts = posStream.count;
        renderableHeader.inputElements = streamcount;
        renderableHeader.streams = vertexStreamCount;

        for (hUint streamIdx = 0; streamIdx < vertexStreamCount; ++streamIdx) {
            hUint64 streamOffset = binoutput->tellp();
            StreamHeader streamHeader = {streamIdx, 0};
            binoutput->write((char*)&streamHeader, sizeof(streamHeader));
            for (hUint vtxIdx = 0; vtxIdx < posStream.count; ++vtxIdx) {
                for (hUint32 inElem = 0; inElem < streamcount; ++inElem) {
                    if (inputDesc[inElem].desc.inputStream_ != streamIdx) {
                        continue;
                    }
                    hUint eleSize=inputDesc[inElem].decodedDataSize/inputDesc[inElem].count;
                    hUint offset=eleSize*vtxIdx;
                    binoutput->write(((char*)inputDesc[inElem].decodedData.get())+offset, eleSize);
                    streamHeader.size+=eleSize;
                }
            }

            binoutput->seekp(streamOffset);
            binoutput->write((char*)&streamHeader, sizeof(streamHeader));
            binoutput->seekp(0, std::ios_base::end);
        }

        binoutput->seekp(writeOffset);
        binoutput->write((char*)&renderableHeader, sizeof(renderableHeader));
        binoutput->seekp(0, std::ios_base::end);
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

