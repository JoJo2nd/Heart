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

DLL_EXPORT
Heart::hResourceClassBase* HEART_API HeartBinLoader( Heart::hISerialiseStream* inStream, Heart::hIDataParameterSet*, Heart::HeartEngine* )
{
    using namespace Heart;

    hFont* resource = hNEW(GetGlobalHeap()/*!heap*/, hFont)();
    hSerialiser ser;
    ser.Deserialise( inStream, *resource );

    return resource;
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

    hFont* font = hNEW(GetGlobalHeap(), hFont)();
    hXMLGetter common = hXMLGetter(&xmldoc).FirstChild("font").FirstChild("common");

    hFloat pageWidth = common.GetAttributeFloat("scaleW");
    hFloat pageHeight = common.GetAttributeFloat("scaleH");
    font->SetFontHeight(common.GetAttributeFloat("lineHeight"));
    font->SetFontWidth(0.f);
    font->SetPageCount(common.GetAttributeInt("pages"));

    hXMLGetter pages = hXMLGetter(&xmldoc).FirstChild("font").FirstChild("pages");
    hXMLGetter page = pages.FirstChild("page");
    const hChar* pageResName = page.GetAttributeString("file", "NONE.NONE");
    hResourceID resLink = hResourceManager::BuildResourceID(pageResName);//TODO:
    font->SetPageResourceID(resLink);
    resLink = hResourceManager::BuildResourceID(hXMLGetter(&xmldoc).FirstChild("font").FirstChild("material").GetAttributeString("resource", "NONE.NONE"));
    font->SetMaterialResourceID(resLink);


    hXMLGetter glyphs = hXMLGetter(&xmldoc).FirstChild("font").FirstChild("chars");
    hUint32 characterCount = glyphs.GetAttributeInt("count");

    font->SetFontCharacterLimit(characterCount);
    
    for (hXMLGetter glyph = glyphs.FirstChild("char"); glyph.ToNode(); glyph = glyph.NextSibling())
    {
        hFontCharacter newchar;
        newchar.page_       = glyph.GetAttributeInt("page", 0);
        newchar.unicode_    = glyph.GetAttributeInt("id", -1);
        newchar.x_          = glyph.GetAttributeInt("x", 0.f);
        newchar.y_          = glyph.GetAttributeInt("y", 0.f);
        newchar.height_     = glyph.GetAttributeInt("width", 0.f);
        newchar.width_      = glyph.GetAttributeInt("height", 0.f);
        newchar.xOffset_    = glyph.GetAttributeInt("xoffset", 0.f);
        newchar.yOffset_    = glyph.GetAttributeInt("yoffset", 0.f);
        newchar.xAdvan_     = glyph.GetAttributeFloat("xadvance", 0.f);

        newchar.UV1_ = hCPUVec2(newchar.x_/pageWidth, 1.f-(newchar.y_/pageHeight));//top left
        newchar.UV2_ = hCPUVec2((newchar.x_+newchar.width_)/pageWidth, 1.f-((newchar.y_+newchar.height_)/pageHeight));//bottom right

        font->AddFontCharacter(&newchar);
    }           

    font->SortCharacters();

    Heart::hSerialiser ser;
    ser.Serialise(binoutput, *font);

    return font;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
hBool HEART_API HeartPackageLink( Heart::hResourceClassBase* resource, Heart::HeartEngine* engine )
{
    using namespace Heart;
    hFont* fnt = static_cast< hFont* >(resource);
    return fnt->Link(engine->GetResourceManager());
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

