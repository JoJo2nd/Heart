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

#include "fontloader.h"
#include <boost/smart_ptr.hpp>
#include <boost/filesystem.hpp>
#include <fstream>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define FONT_MAGIC_NUM              hMAKE_FOURCC('h','F','N','T')
#define FONT_STRING_MAX_LEN         (32)
#define FONT_MAJOR_VERSION          (((hUint16)1))
#define FONT_MINOR_VERSION          (((hUint16)0))
#define FONT_VERSION                ((FONT_MAJOR_VERSION << 16)|FONT_MINOR_VERSION)

//#define FONT_CPP_OUTPUT

#if 0
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT 
void HEART_API HeartGetBuilderVersion(hUint32* verMajor, hUint32* verMinor) {
    *verMajor = 0;
    *verMinor = 9;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
Heart::hResourceClassBase* HEART_API HeartBinLoader( Heart::hISerialiseStream* inStream, Heart::hIDataParameterSet*, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* )
{
    using namespace Heart;
    FontHeader header = {0};

    hFont* font = new memalloc->resourcePakHeap_, hFont)(memalloc->resourcePakHeap_;

    inStream->Read(&header, sizeof(header));

    font->SetFontHeight((hUint32)header.fontHeight);
    font->SetFontWidth(0);
    font->SetPageCount(header.pageCount);
    font->SetPageResourceID(header.pageResID);
    font->SetMaterialResourceID(header.materialResID);
    font->SetFontCharacterLimit(header.glyphCount);

    // A pre-fetch on the inStream of 32K could really speed things up?
    // Should be explicit call on the stream however.

    for (hUint32 i = 0; i < header.glyphCount; ++i)
    {
        hFontCharacter ch;
        inStream->Read(&ch, sizeof(ch));
        font->AddFontCharacter(&ch);
    }

    font->SortCharacters();

    return font;
}
#endif
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int fontCompile(lua_State* L) {
    using namespace Heart;
    using namespace boost;
    /* Args from Lua (1: input files table, 2: dep files table, 3: parameter table, 4: outputpath)*/
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TTABLE);
    luaL_checktype(L, 3, LUA_TTABLE);
    luaL_checktype(L, 4, LUA_TSTRING);

    system::error_code ec;
    hFloat fontScale=1.f;// = hAtoF(params->GetBuildParameter("SCALE", "1"));
    const hChar* cheaderoutput=NULL;
    lua_getfield(L, 3, "scale");
    if (lua_isnumber(L, -1)) {
        fontScale=(hFloat)lua_tonumber(L, -1);
    }
    lua_pop(L, 1);
    lua_getfield(L, 3, "headeroutput");
    if (lua_isstring(L, -1)) {
        cheaderoutput=lua_tostring(L, -1);
    }
    lua_pop(L, 1);

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
    std::ifstream infile;
    rapidxml::xml_document<> xmldoc;

    filesize=(hUint)filesystem::file_size(filepath, ec);
    if (ec) {
        luaL_error(L, "failed to read file size of file %s", filepath);
        return 0;
    }
    infile.open(filepath);
    if (!infile.is_open()) {
        luaL_error(L, "failed to open file %s", filepath);
        return 0;
    }

    scoped_array<hChar> xmlmem(new hChar[filesize+1]);
    memset(xmlmem.get(), 0, filesize);
    infile.read(xmlmem.get(), filesize);
    xmlmem[filesize] = 0;

    try{
        xmldoc.parse< rapidxml::parse_default >(xmlmem.get());
    } catch (rapidxml::parse_error e) {
        luaL_error(L, "Error parsing XML file - %s", e.what());
        return 0;
    } catch (...) {
        luaL_error(L, "Error parsing XML file: Unknown error");
        return 0;
    }
#if 0
    header.resHeader.resourceType = FONT_MAGIC_NUM;
    header.version = FONT_VERSION;

    hXMLGetter common = hXMLGetter(&xmldoc).FirstChild("font").FirstChild("common");

    hFloat pageWidth  = common.GetAttributeFloat("scaleW");
    hFloat pageHeight = common.GetAttributeFloat("scaleH");
    header.fontHeight = common.GetAttributeFloat("lineHeight")*fontScale;
    header.pageCount  = common.GetAttributeInt("pages");

    hXMLGetter pages = hXMLGetter(&xmldoc).FirstChild("font").FirstChild("pages");
    hXMLGetter page = pages.FirstChild("page");
    const hChar* pageResName = page.GetAttributeString("file", "NONE.NONE");
    hResourceID resLink = hResourceManager::BuildResourceID(pageResName);//TODO:
    header.pageResID = resLink;
    resLink = hResourceManager::BuildResourceID(hXMLGetter(&xmldoc).FirstChild("font").FirstChild("material").GetAttributeString("resource", "NONE.NONE"));
    header.materialResID = resLink;

    hXMLGetter glyphs = hXMLGetter(&xmldoc).FirstChild("font").FirstChild("chars");
    hUint32 characterCount = glyphs.GetAttributeInt("count");
    header.glyphCount = characterCount;

    std::ofstream outfile;
    lua_getglobal(L, "buildpathresolve");
    lua_pushvalue(L, 4);
    lua_call(L, 1, 1);
    const hChar* outputpath=lua_tostring(L, -1);
    outfile.open(outputpath, std::ios_base::out|std::ios_base::binary);
    if (!outfile.is_open()) {
        luaL_error(L, "Failed to open output file %s", outputpath);
        return 0;
    }
    outfile.write((char*)&header, sizeof(header));

    std::ofstream headerfile;
    if (cheaderoutput) {
        lua_getglobal(L, "buildpathresolve");
        lua_pushstring(L, cheaderoutput);
        lua_call(L, 1, 1);
        const hChar* headerfilepath=lua_tostring(L, -1);
        headerfile.open(headerfilepath);
        cheaderoutput = headerfile.is_open() ? cheaderoutput : NULL;
        lua_pop(L, 1);
    }
    if (cheaderoutput) {
        headerfile << "/////\n";
        headerfile << "// FontHeader\n";
        headerfile << "const float g_debugfontHeight = " << header.fontHeight << ";\n";
        headerfile << "const unsigned int g_debugpageCount = " << header.pageCount << ";\n";
        headerfile << "const unsigned int g_debugglyphCount = " << header.glyphCount << ";\n";
        headerfile << "\nconst Heart::hFontCharacter g_debugglyphs[] = {\n";
    }

    for (hXMLGetter glyph = glyphs.FirstChild("char"); glyph.ToNode(); glyph = glyph.NextSibling())
    {
        hFontCharacter newchar = {0};
        newchar.page_       = glyph.GetAttributeInt("page", 0);
        newchar.unicode_    = glyph.GetAttributeInt("id", -1);
        newchar.x_          = glyph.GetAttributeFloat("x", 0.f);
        newchar.y_          = glyph.GetAttributeFloat("y", 0.f);
        newchar.height_     = glyph.GetAttributeFloat("height", 0.f);
        newchar.width_      = glyph.GetAttributeFloat("width", 0.f);
        newchar.xOffset_    = glyph.GetAttributeFloat("xoffset", 0.f);
        newchar.yOffset_    = glyph.GetAttributeFloat("yoffset", 0.f);
        newchar.xAdvan_     = glyph.GetAttributeFloat("xadvance", 0.f);

        newchar.UV1_ = hCPUVec2(newchar.x_/pageWidth, ((newchar.y_+newchar.height_)/pageHeight));//top left
        newchar.UV2_ = hCPUVec2((newchar.x_+newchar.width_)/pageWidth, (newchar.y_/pageHeight));//bottom right

        newchar.height_ *= fontScale;
        newchar.width_ *= fontScale;
        newchar.yOffset_ *= fontScale;
        newchar.xOffset_ *= fontScale;
        newchar.xAdvan_ *= fontScale;

        if (cheaderoutput) {
            hChar tmpbuf[1024];
            sprintf_s(tmpbuf, 1024, "{%u, %u, %ff, %ff, %ff, %ff, %ff, %ff, %ff, Heart::hCPUVec2(%ff, %ff), Heart::hCPUVec2(%ff, %ff)},\n",
                newchar.page_, newchar.unicode_, newchar.x_, newchar.y_,
                newchar.height_, newchar.width_, newchar.xOffset_, newchar.yOffset_, 
                newchar.xAdvan_, newchar.UV1_.x, newchar.UV1_.y, newchar.UV2_.x, newchar.UV2_.y);
            headerfile.write(tmpbuf, strlen(tmpbuf));
        }

        outfile.write((char*)&newchar, sizeof(hFontCharacter));
    }

    if (cheaderoutput) {
        headerfile << "}; //const hFontCharacter g_debugglyphs[]\n";
        headerfile.close();
    }

    // link the dependent resource
    lua_newtable(L);
    lua_pushstring(L, pageResName);
    lua_rawseti(L, -2, 1);
    return 1;
#endif
    return 0;
}

#if 0
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
hBool HEART_API HeartPackageLink( Heart::hResourceClassBase* resource, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine )
{
    using namespace Heart;
    hFont* fnt = static_cast< hFont* >(resource);
    return fnt->Link(engine->GetResourceManager());
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

    hFont* font = static_cast<hFont*>(resource);
    delete memalloc->resourcePakHeap_, font;
}
#endif

extern "C" {
//Lua entry point calls
DLL_EXPORT int FB_API luaopen_font(lua_State *L) {
    static const luaL_Reg fontlib[] = {
        {"compile"      , fontCompile},
        {NULL, NULL}
    };
    luaL_newlib(L, fontlib);
    return 1;
}
}

