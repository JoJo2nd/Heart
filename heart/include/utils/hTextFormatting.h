/********************************************************************

    filename: 	hTextFormatting.h	
    
    Copyright (c) 13:12:2012 James Moran
    
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
#pragma once

#ifndef HTEXTFORMATTING_H__
#define HTEXTFORMATTING_H__

namespace Heart
{

enum hFontStyling
{
    hFONT_ALIGN_VCENTRE = 1,
    hFONT_ALIGN_LEFT    = 1 << 1,
    hFONT_ALIGN_RIGHT   = 1 << 2,
    hFONT_ALIGN_TOP     = 1 << 3,
    hFONT_ALIGN_BOTTOM  = 1 << 4,
    hFONT_ALIGN_HCENTRE = 1 << 5,
    hFONT_RIGHT_TO_LEFt = 1 << 6
};

class hFontFormatting
{
public:
    hFontFormatting(hMemoryHeap* heap = GetGlobalHeap())
        : utf8Str_(NULL)
        , strLen_(0)
        , strBytes_(0)
        , flags_(0)
        , outputPtr_(NULL)
        , outputSize_(0)
        , formattedLines_(heap)
    {

    }
    ~hFontFormatting()
    {

    }

    struct hTextExtents
    {
        hFloat width, height;
    };

    void setInputStringBuffer(const hChar* utf8, hUint32 strLen, hUint32 byteLen);
    void setAlignment(hUint32 alignment);
    void setFormatExtents(hFloat maxWidth, hFloat maxHeight);
    void setOutputBuffer(void* buffer, hUint32 bufferLimit);
    void* getOutputBuffer();
    void formatText();
    void writeTextToBuffer();
    hTextExtents getTextExtents();
    hTextExtents getFormatExtents();
    hBool getTextFitsExtents() const;
    hUint32 getLineCount() const;
    hUint32 getPrimitiveCount() const;

private:

    struct hTextLine
    {
        hFloat  startX_;
        hFloat  startY_;
        hFloat  lineWidth_;
        const hChar* lineStart_;
        const hChar* lineEnd_;
    };

    typedef hVector< hTextLine > LineArrayType;

    static hTextLine ProcessLine(const hChar* start, hFloat width, hUint32 formatFlags);

    const hChar* utf8Str_;
    hUint32      strLen_;
    hUint32      strBytes_;
    hUint32      flags_;
    void*        outputPtr_;
    hUint32      outputSize_;
    hTextExtents extents_;

    LineArrayType   formattedLines_;
    hTextExtents    textExtents_;
};

}

#endif // HTEXTFORMATTING_H__