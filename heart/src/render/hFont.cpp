/********************************************************************

    filename: 	hFont.cpp	
    
    Copyright (c) 1:4:2012 James Moran
    
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


namespace Heart
{

struct hFontVex
{
    hCPUVec3    pos_;
    hColour     colour_;
    hCPUVec2    uv_;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hFont::~hFont()
{
    hDELETE_ARRAY_SAFE(heap_, fontCharacters_);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 hFont::RenderString( 
    const hFontStyle& style, 
    void* vBuffer,
    const hCPUVec2& topleft, 
    const hCPUVec2& bottomright, 
    hITextIterator* str)
{
    hFloat scale = style.scale_;
    hUint32 nLines = 0;
    hFloat wid = bottomright.x - topleft.x;
    hFloat hei = topleft.y - bottomright.y;
    hFloat startx;// where to start a line
    hFloat starty;
    hFloat lineinc;// which way a line moves when a new line is added
    hUint32 charsWritten = 0;
    hFloat fheight = (hFloat)style.font_->fontHeight_*scale;

    hcAssert( wid > 0 && hei > 0 );

    void* vtx = vBuffer;

    if ( wid > 0 && hei > 0 ) {
        if ( style.Alignment_ & FONT_ALIGN_LEFT ) {
            startx = topleft.x;
        }
        else if ( style.Alignment_ & FONT_ALIGN_RIGHT ) {
            startx = bottomright.x;
        }
//         else if ( style.Alignment_ & FONT_ALIGN_HCENTRE ) {
//             startx = topleft.x + ( wid / 2 );
//         }
        else {//Cases that aren't handled yet
            hcBreak;
            return 0;
        }

        if ( style.Alignment_ & FONT_ALIGN_TOP ) {
            starty = topleft.y;
            lineinc = -fheight;
        }
        else if ( style.Alignment_ & FONT_ALIGN_BOTTOM ) {
            starty = bottomright.y;
            lineinc = fheight;
        }
        else {//Cases that aren't handled yet
            hcBreak;
            return 0;
        }

        hFloat texth = fheight * nLines;
        hFloat starty;

        if ( style.Alignment_ & FONT_ALIGN_BOTTOM ) {
            starty = bottomright.y;
        }
        else if ( style.Alignment_ & FONT_ALIGN_TOP ) {
            starty = topleft.y;
        }
        else {
            hcBreak;//TODO:
        }

        for (; str->getCharCode(); ) {
            hUint32 bytesWritten;
            hUint32 written = RenderStringSingleLine(style, vtx, hCPUVec2(topleft.x, starty), str, &bytesWritten, wid);
            starty += lineinc;
            vtx = ((hByte*)vtx)+ bytesWritten;
            charsWritten += written;
        }
    }
    
    return charsWritten * 2;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 hFont::RenderStringSingleLine(
    const hFontStyle& style, 
    void* vBuffer, 
    const hCPUVec2& topleft, 
    hITextIterator* str,
    hUint32* bytesWritten,
    hFloat widthLimit /*= FLT_MAX*/)
{
    HEART_PROFILE_FUNC();
    hUint32 nLines = 0;
    hFloat startx;
    hFloat starty;
    hUint32 charsWritten = 0;
    hUint16 vOffset = 0;
    hFloat fh = (hFloat)style.font_->fontHeight_;
    hBool doneOneWord = hFalse;
    hBool whitespacePrev = hFalse;
    hITextIterator::hTextMarker lastMkr;
    hFontVex* vtx = (hFontVex*)vBuffer;

    lastMkr = str->createMarker();
    startx = topleft.x;
    starty = topleft.y;
    widthLimit += startx;

    for (;str->getCharCode() && (startx < widthLimit || !doneOneWord);str->next()) {
        const hFontCharacter& c = *style.font_->GetFontCharacter(str->getCharCode());
        hChar ac = (str->getCharCode() & ~0x7F) == 0 ? (0x7F & str->getCharCode()) : 0;

        // line break checking, gotta be ascii character for this
        if (ac && hIsSpace(ac)) {
            if (ac == ' ') {
                doneOneWord = hTrue;
                startx += c.xAdvan_*style.scale_;
            }
            else if (ac == '\t') {
                startx += c.xAdvan_*style.scale_*2;
            }
            else {
                str->next();
                break;//hit return
            }
            whitespacePrev = hTrue;
            continue;
        }
        else if (whitespacePrev) {
            lastMkr = str->createMarker();
        }
        
        if (ac && isprint(ac) == 0) continue;

        // ordered top left, top right, bottom left, bottom right
        hFloat h1 = (c.yOffset_)*style.scale_;
        hFloat h2 = (c.yOffset_+c.height_)*style.scale_;
        hFloat w1 = (c.xOffset_)*style.scale_;
        hFloat w2 = (c.xOffset_ + c.width_)*style.scale_;
        hFontVex quad[ 4 ] = {
            { hCPUVec3( startx + w1             , starty+h1, 0.0f ), style.Colour_, c.UV1_ },
            { hCPUVec3( startx + w2             , starty+h1, 0.0f ), style.Colour_, hCPUVec2( c.UV2_.x, c.UV1_.y ) },
            { hCPUVec3( startx + w1             , starty+h2, 0.0f ), style.Colour_, hCPUVec2( c.UV1_.x, c.UV2_.y ) },
            { hCPUVec3( startx + w2             , starty+h2, 0.0f ), style.Colour_, c.UV2_ },
        };

        *vtx = quad[ 0 ]; ++vtx;
        *vtx = quad[ 1 ]; ++vtx;
        *vtx = quad[ 2 ]; ++vtx;

        *vtx = quad[ 2 ]; ++vtx;
        *vtx = quad[ 1 ]; ++vtx;
        *vtx = quad[ 3 ]; ++vtx;

        startx += c.xAdvan_*style.scale_;
        whitespacePrev = hFalse;
        ++charsWritten;
    }

    if (bytesWritten) *bytesWritten = charsWritten*12*sizeof(hFontVex);
    return charsWritten*2;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool hFont::FitLine( Private::hFontLine& line, hFloat wid, const hChar* pStr )
{
    return hFalse;
    /*hBool canfit = hTrue;
    hBool mark = hFalse;
    hBool space = hFalse;
    line.pStart_ = pStr;
    line.pEnd_ = NULL;
    line.Width_ = 0.0f;
    const hChar* pCur = line.pStart_;
    hFloat lwid = 0.0f;
 
    while( canfit || line.pEnd_ == NULL )// MUST fit one word on each line [2/26/2009 James]
    {
        if ( *pCur == 0 ) 
        {
            line.pEnd_ = pCur;
            line.Width_ = lwid;
            return hFalse;
        }
 
        if ( *pCur == '\n' )
        {
            //skip and end line
            ++pCur;
            line.pEnd_ = pCur;
            line.Width_ = lwid;
            return hTrue;
        }
 
        if ( *pCur == ' ' )
        {
            if ( line.pEnd_ && !space )
            {
                line.Width_ = lwid;
                line.pEnd_ = pCur;
            }
            else
            {
                // trim leading spaces [2/26/2009 James]
                while( *pCur == ' ' ) ++pCur;
 
                if( !mark )
                    line.pStart_ = pCur;
                line.pEnd_ = pCur;

                if ( *pCur == '\0' )
                {
                    // end of string
                    line.Width_ = lwid;
                    return hFalse;
                }

                continue;
            }
 
            space = hTrue;
            ++pCur;
            continue;
        }
 
        if ( *pCur == '\t' )
        {
            ++pCur;
            continue;
        }

        space = hFalse;
 
        const hFontCharacter& c = *GetFontCharacter( *pCur );
        if ( c.unicode_ != -1 )
        {
            lwid += (hFloat)c.xAdvan_;
        }
        else if ( *pCur != ' ' )
        {
            lwid += (hFloat)c.xAdvan_;
 #ifdef HEART_PRINT_UNKNOWN_CHAR_CODES
            hcPrintf( "unknown char %c code %d\n", *pCur, *pCur );
 #endif
        }
 
        if ( lwid > wid )
        { 
            canfit = hFalse;
        }
 
        ++pCur;
        mark = hTrue;
    }
 
    return hTrue;*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

hFontCharacter* hFont::GetFontCharacter( hUint32 charcode )
{
    HEART_PROFILE_FUNC();
    //binary search for the character
//  	hInt32 top = nFontCharacters_;
//  	hInt32 bottom = 0;
//  	hInt32 mid;
//  
//  	while ( top >= bottom )
//  	{
//  		mid = (bottom + top)/ 2;
//  		if ( fontCharacters_[mid].unicode_ > charcode )
//  		{
//  			top = mid-1;
//  		}
//  		else if ( fontCharacters_[mid].unicode_ < charcode )
//  		{
//  			bottom = mid+1;
//  		}
//  		else
//  		{
//  			return &fontCharacters_[mid];
//  		}
//  	}
//  
//  	return &fontCharacters_[nFontCharacters_-1];
    
    // "Step" search. Most fonts contain blocks of character (e.g. think ascii range)
    // We abuse that fact with this search, because when fons get big e.g. > 1000 glphys 
    // a binary search doesn't cut it.
    for (hUint32 i = 1; i < fontLookupSteps_; ++i)
    {
        if (charcode >= (fontLookup_[i-1]&0xFFFFFFFF) && charcode <= (fontLookup_[i]&0xFFFFFFFF))
            return &fontCharacters_[charcode-((fontLookup_[i-1]&0xFFFFFFFF00000000)>>32)];
    }
    
    return &fontCharacters_[nFontCharacters_-1];
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hCPUVec2 hFont::CalcRenderSize(hITextIterator* str)
{
    HEART_PROFILE_FUNC();
    hCPUVec2 ret(0.f,0.f);
    for (;str->getCharCode();str->next())
    {
        const hFontCharacter& fchar = *GetFontCharacter(str->getCharCode());
        ret.x += fchar.xAdvan_;
    }

    return ret;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hFont::SetFontCharacterLimit( hUint32 nChars )
{
    nMaxFontCharacters_ = nChars;
    hDELETE_ARRAY_SAFE(heap_, fontCharacters_);
    hDELETE_ARRAY_SAFE(heap_, fontLookup_);
    fontCharacters_ = hNEW_ARRAY(heap_, hFontCharacter, nMaxFontCharacters_);
    fontLookup_ = hNEW_ARRAY(heap_, hFontLookup, nMaxFontCharacters_);//TODO: reduce this
    nFontCharacters_ = 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hFont::AddFontCharacter( const hFontCharacter* fchar )
{
    hcAssert(nFontCharacters_ < nMaxFontCharacters_);
    fontLookup_[nFontCharacters_] = fchar->unicode_;
    fontCharacters_[nFontCharacters_++] = *fchar;
}

int hFontCharacterSort(const void* lhs, const void* rhs)
{
    return ((hFontCharacter*)lhs)->unicode_ < ((hFontCharacter*)rhs)->unicode_ ? -1 : 1;
}

int hFontIDSort(const void* lhs, const void* rhs)
{
    return *((hFontLookup*)lhs) < *((hFontLookup*)rhs) ? -1 : 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hFont::SortCharacters()
{
    qsort(fontCharacters_, nFontCharacters_, sizeof(hFontCharacter), hFontCharacterSort);
    hUint32 of = 0;
    hUint32 nSteps = 0;
    for (hUint32 i = 0; i < nFontCharacters_; ++i)
    {
        if (fontCharacters_[i].unicode_ > (of+1))
        {
            fontLookup_[nSteps++] = ((((hUint64)fontCharacters_[i].unicode_-i)<<32) | fontCharacters_[i].unicode_);
        }
        of = fontCharacters_[i].unicode_;
    }
    //fontLookup_[nSteps-1].idx_ = nFontCharacters_-2;//-2 because last entry is invalid character
    //fontLookup_[nSteps-1].unicode_ = fontCharacters_[nFontCharacters_-2].unicode_;
    fontLookup_[nSteps-1] = ((((hUint64)nFontCharacters_-2)<<32) | fontCharacters_[nFontCharacters_-2].unicode_);
    fontLookupSteps_ = nSteps;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool hFont::Link( hResourceManager* resManager )
{
    texturePages_ = static_cast<hTexture*>(resManager->ltGetResource(texturePageResID_));    
    fontMaterial_ = static_cast<hMaterial*>(resManager->ltGetResource(fontMaterialID_));

    if (texturePages_ && fontMaterial_)
    {
        //fontMaterialInstance_ = fontMaterial_->CreateMaterialInstance();
        return hTrue;
    }
    return hFalse;
}

}