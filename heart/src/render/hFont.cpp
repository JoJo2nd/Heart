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

hFont::~hFont()
{
    hDELETE_ARRAY_SAFE(heap_, fontCharacters_);
    hDELETE_ARRAY_SAFE(heap_, fontLookup_);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

const hFontCharacter* hFont::GetFontCharacter(hUint32 charcode) const
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