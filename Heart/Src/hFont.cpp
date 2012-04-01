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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 hFont::RenderString( hIndexBuffer& iBuffer, 
							hVertexBuffer& vBuffer, 
							const hCPUVec2& topleft, 
							const hCPUVec2& bottomright, 
							const hChar* str,
							hRenderSubmissionCtx* rnCtx )
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

    hIndexBufferMapInfo  ibMap;
    hVertexBufferMapInfo vbMap;
    rnCtx->Map( &iBuffer, &ibMap );
    rnCtx->Map( &vBuffer, &vbMap );

    hUint16* idx = (hUint16*)ibMap.ptr_;
    void* vtx = vbMap.ptr_;

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

    rnCtx->Unmap( &ibMap );
    rnCtx->Unmap( &vbMap );

    rnCtx->SetMaterialInstance( fontMaterial_ );
    hUint32 passes = rnCtx->GetMaterialInstancePasses();
    for ( hUint32 i = 0; i < passes; ++i )
    {
        rnCtx->BeingMaterialInstancePass( i );

        rnCtx->SetVertexStream( 0, &vBuffer );
        rnCtx->SetIndexStream( &iBuffer );

        rnCtx->DrawIndexedPrimitive( charsWritten * 2, 0 );
        rnCtx->EndMaterialInstancePass();
    }

// 	if ( pCmdList )
// 	{
// 		//hcAssertMsg( pDrawMaterial, "Must give a material to render the text with" );
// 		hcAssert( startOffset % 3 == 0 );//sanity check
// 
// 		//cleart the prev list
// 		pCmdList->Clear();
// 
// 		// addative blend
// 		// material should handle blend
// 		//pCmdList->NewRenderCommand< Cmd::SetAlphaBlend >( RSV_ENABLE, RSV_BLEND_FUNC_ADD, RSV_BLEND_OP_SRC_ALPHA, RSV_BLEND_OP_INVSRC_ALPHA );
// 
// 		pCmdList->NewRenderCommand< Cmd::SetStreams >( &iBuffer, &vBuffer );
// 		pCmdList->NewRenderCommand< Cmd::DrawPrimative >( charsWritten * 2 + ( startOffset / 3 ) );
// 	}
	
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
 
 		if ( space == hTrue )
 		{
 			lwid += spaceWidth_;
 			space = hFalse;
 		}
 
		const Private::hFontCharacter& c = *GetFontCharacter( *pCur );
 		if ( c.CharCode_ != -1 )
 		{
 			lwid += (hFloat)c.xAdvan_;
 		}
 		else if ( *pCur != ' ' )
 		{
 			lwid += spaceWidth_;
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
	hFloat boty = cury - fontHeight_ + baseLine_;

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
		if ( *pstr == ' ' )
		{
			startx += spaceWidth_;
			++pstr;
			continue;
		}

		const Private::hFontCharacter& c = *GetFontCharacter( *pstr );

		struct Vex
		{
			hCPUVec3	pos_;
			hColour		colour_;
			hCPUVec2	uv_;
		};

		// ordered top left, top right, bottom left, bottom right
		Vex quad[ 4 ] = 
		{
			{ hCPUVec3( startx			    , boty + c.Height_ + c.BaseLine_, 0.0f ), style_.Colour_, c.UV1_ },
			{ hCPUVec3( startx + c.Width_	, boty + c.Height_ + c.BaseLine_, 0.0f ), style_.Colour_, hCPUVec2( c.UV2_.x, c.UV1_.y ) },
			{ hCPUVec3( startx			    , boty + c.BaseLine_			, 0.0f ), style_.Colour_, hCPUVec2( c.UV1_.x, c.UV2_.y ) },
			{ hCPUVec3( startx + c.Width_	, boty + c.BaseLine_			, 0.0f ), style_.Colour_, c.UV2_ },
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

/*
		iBuffer.SetIndex( iOffset++, (hUint16)vOffset   );
		iBuffer.SetIndex( iOffset++, (hUint16)vOffset+2 );
		iBuffer.SetIndex( iOffset++, (hUint16)vOffset+1 );

		iBuffer.SetIndex( iOffset++, (hUint16)vOffset+2 );
		iBuffer.SetIndex( iOffset++, (hUint16)vOffset+3 );
		iBuffer.SetIndex( iOffset++, (hUint16)vOffset+1 );

		for ( hUint32 i = 0; i < 4; ++i )
		{
			vBuffer.SetElement( vOffset, hrVE_XYZ, quad[ i ].pos_ );
			vBuffer.SetElement( vOffset, hrVE_COLOR, quad[ i ].colour_ );
			vBuffer.SetElement( vOffset, hrVE_1UV, quad[ i ].uv_ );
			++vOffset;
		}
*/
		startx += c.xAdvan_;

		++charsWritten;

		++pstr;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

hResourceClassBase* hFont::OnFontLoad( const hChar* ext, hUint32 resID, hSerialiserFileStream* dataStream, hResourceManager* resManager )
{
    hFont* resource = hNEW(hGeneralHeap, hFont);
    hSerialiser ser;
    ser.Deserialise( dataStream, *resource );

    resManager->GetRederer()->CreateTexture( 
        resource->fontSourceWidth_, 
        resource->fontSourceHeight_, 
        0, 
        (void*)resource->fontSourceData_, 
        resource->fontSourceSize_, 
        TFORMAT_ARGB8, 
        RESOURCEFLAG_RENDERTARGET, 
        &resource->texturePages_ );

    resource->fontMaterial_ = resManager->GetMaterialManager()->CreateMaterialInstance( (hUint32)resource->fontMaterial_ );
    const hSamplerParameter* samp = resource->fontMaterial_->GetSamplerParameterByName( "ColorSampler" );
    resource->fontMaterial_->SetSamplerParameter( samp, resource->texturePages_ );

 	return resource;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

hUint32	 hFont::OnFontUnload( const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager )
{
    delete resource;
 	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

Private::hFontCharacter* hFont::GetFontCharacter( hUint32 charcode )
{
	//binary search for the character
	hUint32 top = nFontCharacters_-1;
	hUint32 bottom = 0;
	hUint32 mid;

	do
	{
		mid = bottom + ( (top - bottom) / 2 );
		if ( fontCharacters_[ mid ].CharCode_ > charcode )
		{
			top = mid-1;
		}
		else if ( fontCharacters_[ mid ].CharCode_ < charcode )
		{
			bottom = mid+1;
		}
		else
		{
			return &fontCharacters_[ mid ];
		}
	}
	while( bottom <= top );

	return &fontCharacters_[ 0 ];
}

}