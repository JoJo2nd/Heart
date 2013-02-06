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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define FONT_MAGIC_NUM              hMAKE_FOURCC('h','F','N','T')
#define FONT_STRING_MAX_LEN         (32)
#define FONT_MAJOR_VERSION          (((hUint16)1))
#define FONT_MINOR_VERSION          (((hUint16)0))
#define FONT_VERSION                ((FONT_MAJOR_VERSION << 16)|FONT_MINOR_VERSION)

//#define FONT_CPP_OUTPUT

#pragma pack(push, 1)

struct FontHeader
{
    Heart::hResourceBinHeader   resHeader;
    hUint32                     version;
    hFloat                      fontHeight;
    hUint32                     pageCount;
    Heart::hResourceID          pageResID;
    Heart::hResourceID          materialResID;
    hUint32                     glyphCount;
};

#pragma pack(pop)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
Heart::hResourceClassBase* HEART_API HeartBinLoader( Heart::hISerialiseStream* inStream, Heart::hIDataParameterSet*, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* )
{
    using namespace Heart;
    FontHeader header = {0};

    hFont* font = hNEW(memalloc->resourcePakHeap_, hFont)(memalloc->resourcePakHeap_);

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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
hBool HEART_API HeartDataCompiler( Heart::hIDataCacheFile* inFile, Heart::hIBuiltDataCache* fileCache, Heart::hIDataParameterSet* params, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine, Heart::hISerialiseStream* binoutput )
{
    using namespace Heart;
    FontHeader header = {0};
    hFloat fontScale = hAtoF(params->GetBuildParameter("SCALE", "1"));
    hXMLDocument xmldoc;
    hChar* xmlmem = (hChar*)hHeapMalloc(memalloc->tempHeap_, inFile->Lenght()+1);
    inFile->Read(xmlmem, inFile->Lenght());
    xmlmem[inFile->Lenght()] = 0;

    if (xmldoc.ParseSafe< rapidxml::parse_default >(xmlmem, memalloc->tempHeap_) == hFalse)
        return NULL;

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
    binoutput->Write(&header, sizeof(header));

#ifdef FONT_CPP_OUTPUT
    hcPrintf("/////\n");
    hcPrintf("// FontHeader\n");
    hcPrintf("const float g_debugfontHeight = %ff;\n", header.fontHeight);
    hcPrintf("const unsigned int g_debugpageCount = %d;\n", header.pageCount);
    hcPrintf("const unsigned int g_debugglyphCount = %d;\n", header.glyphCount);
    hcPrintf("\nconst Heart::hFontCharacter g_debugglyphs[] = {\n");
#endif

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

#ifdef FONT_CPP_OUTPUT
        hcPrintf("{%u, %u, %ff, %ff, %ff, %ff, %ff, %ff, %ff, Heart::hCPUVec2(%ff, %ff), Heart::hCPUVec2(%ff, %ff)},",
            newchar.page_, newchar.unicode_, newchar.x_, newchar.y_,
            newchar.height_, newchar.width_, newchar.xOffset_, newchar.yOffset_, 
            newchar.xAdvan_, newchar.UV1_.x, newchar.UV1_.y, newchar.UV2_.x, newchar.UV2_.y);
#endif

        binoutput->Write(&newchar, sizeof(hFontCharacter));
    }

#ifdef FONT_CPP_OUTPUT
    hcPrintf("}; //const hFontCharacter g_debugglyphs[]\n");
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
    hDELETE(memalloc->resourcePakHeap_, font);
}

