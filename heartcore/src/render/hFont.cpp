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

    struct Vex
    {
        hCPUVec3	pos_;
        hColour		colour_;
        hCPUVec2	uv_;
    };

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hFont::~hFont()
{
    hDELETE_ARRAY_SAFE(GetGlobalHeap()/*!heap*/, fontCharacters_);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 hFont::RenderString( void* iBuffer, 
							 void* vBuffer, 
							 const hCPUVec2& topleft, 
							 const hCPUVec2& bottomright, 
							 const hChar* str)
{

	//hChar pFormatedString[ 4098 ];
	Private::hFontLine Lines[ 256 ];
	hUint32 nLines = 0;
	hFloat wid = bottomright.x - topleft.x;
	hFloat hei = topleft.y - bottomright.y;
	hFloat startx;// where to start a line [2/26/2009 James]
	hFloat starty;
	hFloat lineinc;// which way a line moves when a new line is added [2/26/2009 James]
	hUint32 charsWritten = 0;

	const hChar* pforstr = str;

	hcAssert( wid > 0 && hei > 0 );

	hUint16 iOffset = 0;//iBuffer.GetIndexCount(); 
	hUint16 vOffset = 0;//vBuffer.VertexCount();
	hUint32 startOffset = iOffset;

    hUint16* idx = (hUint16*)iBuffer;
    void* vtx = vBuffer;

	if ( wid > 0 && hei > 0 )
	{
		if ( style_.Alignment_ & FONT_ALIGN_LEFT )
		{
			startx = topleft.x;
		}
		else if ( style_.Alignment_ & FONT_ALIGN_RIGHT )
		{
			startx = bottomright.x;
		}
		else if ( style_.Alignment_ & FONT_ALIGN_HCENTRE )
		{
			startx = topleft.x + ( wid / 2 );
		}
		else
		{
			hcBreak;// need to set font alignment [2/26/2009 James]
			return 0;
		}

		if ( style_.Alignment_ & FONT_ALIGN_TOP )
		{
			starty = topleft.y;
			lineinc = -(hFloat)fontHeight_;
		}
		else if ( style_.Alignment_ & FONT_ALIGN_BOTTOM )
		{
			starty = bottomright.y;
			lineinc = (hFloat)fontHeight_;
		}
		// centering is a special case [2/26/2009 James]
		
		// build a list of lines to draw [2/26/2009 James]
		const hChar* pStr = pforstr;
		for ( hUint32 i = 0; i < 255 && FitLine( Lines[ i ], wid, pStr ); )
		{
			pStr = Lines[ i ].pEnd_;
			// fit line can return empty lines, we want to skip printing these lines
			if ( Lines[ i ].pStart_ != Lines[ i ].pEnd_ )
			{
				++nLines;
				++i;
			}
		}
		++nLines;

		hFloat texth = (hFloat)fontHeight_ * nLines;
		hFloat starty;

		if ( style_.Alignment_ & FONT_ALIGN_BOTTOM )
		{
			starty = bottomright.y;
		}
		else if ( style_.Alignment_ & FONT_ALIGN_TOP )
		{
			starty = topleft.y;
		}
		else if ( style_.Alignment_ & FONT_ALIGN_VCENTRE )
		{
			starty = topleft.y - ( hei / 2 );
			starty += ( texth / 2.0f );
		}

		if ( lineinc < 0 )
		{
			for ( hUint32 i = 0; i < nLines; ++i )
			{
				RenderLine( &idx, &vtx, vOffset, Lines[ i ], starty, topleft, bottomright, wid, charsWritten );
				starty += lineinc;
			}
		}
		else
		{
			for ( hUint32 i = nLines - 1; i < nLines; --i )
			{
				RenderLine( &idx, &vtx, vOffset, Lines[ i ], starty, topleft, bottomright, wid, charsWritten );
				starty += lineinc;
			}
		}
	}
	
	return charsWritten * 2;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 hFont::RenderStringSingleLine(void* iBuffer, void* vBuffer, const hCPUVec2& topleft, const hChar* str)
{
    hUint32 nLines = 0;
    hFloat startx;
    hFloat starty;
    hUint32 charsWritten = 0;
    const hChar* pforstr = str;
    hUint16 iOffset = 0;//iBuffer.GetIndexCount(); 
    hUint16 vOffset = 0;//vBuffer.VertexCount();
    hFloat fh = (hFloat)fontHeight_;

    hUint16* idx = (hUint16*)iBuffer;
    Vex* vtx = (Vex*)vBuffer;

    startx = topleft.x;
    starty = topleft.y;

    for (;*str;++str)
    {
        const hFontCharacter& c = *GetFontCharacter( *str );

        // ordered top left, top right, bottom left, bottom right
        hFloat h1 = /*(style_.Alignment_ & FONT_ALIGN_FLIP) ?*/ (c.yOffset_+c.height_)            ;// : 0;
        hFloat h2 = /*(style_.Alignment_ & FONT_ALIGN_FLIP) ?*/ (c.yOffset_)  ;// : c.height_;
        Vex quad[ 4 ] = 
        {
            { hCPUVec3( startx + c.xOffset_			    , starty+h1, 0.0f ), style_.Colour_, c.UV1_ },
            { hCPUVec3( startx + c.xOffset_ + c.width_	, starty+h1, 0.0f ), style_.Colour_, hCPUVec2( c.UV2_.x, c.UV1_.y ) },
            { hCPUVec3( startx + c.xOffset_             , starty+h2, 0.0f ), style_.Colour_, hCPUVec2( c.UV1_.x, c.UV2_.y ) },
            { hCPUVec3( startx + c.xOffset_ + c.width_	, starty+h2, 0.0f ), style_.Colour_, c.UV2_ },
        };

        ///////////////////////////////////////////////////////////////////////////////////////////////////
        // Draw as a quad /////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        //hUint16* idx = *idx;
        //Vex* vtx = (Vex*)*vBuffer;
        *idx = vOffset;   ++idx;
        *idx = vOffset+2; ++idx;
        *idx = vOffset+1; ++idx;

        *idx = vOffset+2; ++idx;
        *idx = vOffset+3; ++idx;
        *idx = vOffset+1; ++idx;

        vOffset += 4;

        for ( hUint32 i = 0; i < 4; ++i )
        {
            *vtx = quad[ i ]; ++vtx;
        }

        startx += c.xAdvan_;

        ++charsWritten;

    }

    return charsWritten * 2;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool hFont::FitLine( Private::hFontLine& line, hFloat wid, const hChar* pStr )
{
 	hBool canfit = hTrue;
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
 
 	return hTrue;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hFont::RenderLine( hUint16** iBuffer, void** vBuffer, hUint16& vOffset, Private::hFontLine& line, hFloat cury, const hCPUVec2& topleft, const hCPUVec2& bottomright, hFloat w, hUint32& charsWritten )
{
	hFloat startx;
	hFloat boty = cury - fontHeight_ /*+ baseLine_*/;

	if ( style_.Alignment_ & FONT_ALIGN_LEFT )
	{
		startx = topleft.x;
	}
	else if ( style_.Alignment_ & FONT_ALIGN_RIGHT )
	{
		startx = bottomright.x - line.Width_;
	}
	else if ( style_.Alignment_ & FONT_ALIGN_HCENTRE )
	{
		startx = topleft.x + ( w / 2.0f );
		startx -= line.Width_ / 2.0f;
	}

	const hChar* pstr = line.pStart_;
	while ( pstr != line.pEnd_ )
	{
		if ( *pstr == '\n' || *pstr == '\t' || *pstr == 0 )
		{
			++pstr;
			continue;
		}

		const hFontCharacter& c = *GetFontCharacter( *pstr );

		struct Vex
		{
			hCPUVec3	pos_;
			hColour		colour_;
			hCPUVec2	uv_;
		};

		// ordered top left, top right, bottom left, bottom right
		Vex quad[ 4 ] = 
		{
			{ hCPUVec3( startx			    , boty + c.height_ /*+ c.BaseLine_*/, 0.0f ), style_.Colour_, c.UV1_ },
			{ hCPUVec3( startx + c.width_	, boty + c.height_ /*+ c.BaseLine_*/, 0.0f ), style_.Colour_, hCPUVec2( c.UV2_.x, c.UV1_.y ) },
			{ hCPUVec3( startx			    , boty /*+ c.BaseLine_*/			, 0.0f ), style_.Colour_, hCPUVec2( c.UV1_.x, c.UV2_.y ) },
			{ hCPUVec3( startx + c.width_	, boty /*+ c.BaseLine_*/			, 0.0f ), style_.Colour_, c.UV2_ },
		};

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// Draw as a quad /////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////
		hUint16* idx = *iBuffer;
        Vex* vtx = (Vex*)*vBuffer;
        *idx = vOffset;   ++idx;
        *idx = vOffset+2; ++idx;
        *idx = vOffset+1; ++idx;

        *idx = vOffset+2; ++idx;
        *idx = vOffset+3; ++idx;
        *idx = vOffset+1; ++idx;

        for ( hUint32 i = 0; i < 4; ++i )
        {
	        *vtx = quad[ i ]; ++vtx;
            ++vOffset;
        }

        *iBuffer = idx;
        *vBuffer = vtx;

		startx += c.xAdvan_;

		++charsWritten;

		++pstr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

hFontCharacter* hFont::GetFontCharacter( hUint32 charcode )
{
	//binary search for the character
	hInt32 top = nFontCharacters_;
	hInt32 bottom = 0;
	hInt32 mid;

	while ( top >= bottom )
	{
		mid = (bottom + top)/ 2;
		if ( fontCharacters_[mid].unicode_ > charcode )
		{
			top = mid-1;
		}
		else if ( fontCharacters_[mid].unicode_ < charcode )
		{
			bottom = mid+1;
		}
		else
		{
			return &fontCharacters_[mid];
		}
	}

	return &fontCharacters_[nFontCharacters_-1];
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hCPUVec2 hFont::CalcRenderSize( const hChar* str )
{
    hCPUVec2 ret(0.f,0.f);
    for (;*str;++str)
    {
        const hFontCharacter& fchar = *GetFontCharacter(*str);
        ret.x += fchar.xAdvan_;
        ret.y = hMax(ret.y, fontHeight_);
    }

    return ret;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hFont::SetFontCharacterLimit( hUint32 nChars )
{
    nMaxFontCharacters_ = nChars;
    if (fontCharacters_)
    {
        hDELETE_ARRAY_SAFE(GetGlobalHeap(), fontCharacters_);
    }
    fontCharacters_ = hNEW_ARRAY(GetGlobalHeap(), hFontCharacter, nMaxFontCharacters_);
    nFontCharacters_ = 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hFont::AddFontCharacter( const hFontCharacter* fchar )
{
    hcAssert(nFontCharacters_ < nMaxFontCharacters_);
    fontCharacters_[nFontCharacters_++] = *fchar;
}

int hFontCharacterSort(const void* lhs, const void* rhs)
{
    return ((hFontCharacter*)lhs)->unicode_ < ((hFontCharacter*)rhs)->unicode_ ? -1 : 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hFont::SortCharacters()
{
    qsort(fontCharacters_, nFontCharacters_, sizeof(hFontCharacter), hFontCharacterSort);
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
        fontMaterialInstance_ = fontMaterial_->CreateMaterialInstance();
        return hTrue;
    }
    return hFalse;
}

}