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

class hFont;

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
    hFontFormatting(hMemoryHeapBase* heap = GetGlobalHeap())
        : inputUTF8Str_(heap)
        , flags_(0)
        , font_(0)
        , scale_(1.f)
        , outputPtr_(NULL)
        , outputSize_(0)
        , formattedLines_(heap)
        , primCount_(0)
    {
    }
    ~hFontFormatting()
    {

    }

    struct hTextExtents
    {
        hFloat width, height;
    };

    void setInputStringBuffer(const hChar* utf8, hUint32 byteLen)
    {
        inputUTF8Str_.Resize(1);
        inputUTF8Str_[0].bytes_ = byteLen;
        inputUTF8Str_[0].str_ = utf8;
    }
    void setInputStringBuffers(const hChar** utf8, hUint32* byteLen, hUint32 bufferCount)
    {
        inputUTF8Str_.Resize(bufferCount);
        for (hUint32 i = 0; i < bufferCount; ++i)
        {
            inputUTF8Str_[i].bytes_ = byteLen[i];
            inputUTF8Str_[i].str_ = utf8[i];
        }
    }
    void setAlignment(hUint32 alignment) { flags_ = alignment; }
    void setFormatExtents(hFloat maxWidth, hFloat maxHeight)
    {
        extents_.width = maxWidth;
        extents_.height = maxHeight;
    }
    void setOutputBuffer(void* buffer, hUint32 bufferLimit) 
    {
        outputPtr_ = buffer;
        outputSize_ = bufferLimit;
    }
    void* getOutputBuffer() { return outputPtr_; }
    void setFont(hFont* font) { font_ = font; }
    void setScale(hFloat scale) { scale_ = scale; }
    void setColour(hColour col) { colour_ = col; }
    void setReserve(hUint32 lines) { formattedLines_.Reserve(lines); }
    void formatText();
    void writeTextToBuffer(const hCPUVec2& topleft);
    hTextExtents getTextExtents() const { return textExtents_; }
    hTextExtents getFormatExtents() const { return extents_; }
    hBool getTextFitsExtents() const { return textExtents_.height < extents_.height; }
    hUint32 getLineCount() const { return formattedLines_.GetSize(); }
    hUint32 getPrimitiveCount() const { return primCount_; }

private:

    struct hStringInputBuffer
    {
        hUint32 bytes_;
        const hChar* str_;
    };

    class hTextIterator
    {
    public:
        hTextIterator()
            : formatter_(NULL)
            , strBufIdx_(0)
            , strOs_(0)
            , value_(0)
        {
        }
        hTextIterator(hFontFormatting* formatter)
            : formatter_(formatter)
            , strBufIdx_(0)
            , strOs_(0)
            , value_(0)
        {
            hChar tmputf[4];
            hUTF8::Unicode cc;
            if (strBufIdx_ >= formatter_->inputUTF8Str_.GetSize()) {
                return;
            }
            hStringInputBuffer ib = formatter_->inputUTF8Str_[strBufIdx_];
            hUint32 adv = hUTF8::BytesInUTF8Character(ib.str_+strOs_);
            if (adv > (ib.bytes_-strOs_)) {
                hStringInputBuffer nib = formatter_->inputUTF8Str_[strBufIdx_];
                for (hUint32 i = strOs_, c = strOs_+adv; i < c; ++i) {
                    tmputf[i] = (strOs_+i < ib.bytes_) ? ib.str_[i] : nib.str_[i-strOs_];
                }
                hUTF8::DecodeToUnicode(tmputf, cc);
            }
            else {
                hUTF8::DecodeToUnicode(ib.str_+strOs_, cc);
                value_ = cc;
            }
            value_ = cc;
        }
        hTextIterator(const hTextIterator& rhs) {
            formatter_ = rhs.formatter_;
            strBufIdx_ = rhs.strBufIdx_;
            strOs_ = rhs.strOs_;
            value_ = rhs.value_;
        }
        ~hTextIterator() {}
        hTextIterator& operator = (const hTextIterator& rhs) { 
            formatter_ = rhs.formatter_;
            strBufIdx_ = rhs.strBufIdx_;
            strOs_ = rhs.strOs_;
            value_ = rhs.value_;
            return *this; }
        hTextIterator& operator ++ () { 
            next();
            return *this;
        }
        hUint32 operator * () { return value_; }
        hBool operator == (const hTextIterator& rhs) { return equal(rhs); }
        hBool operator != (const hTextIterator& rhs) { return !equal(rhs); }
        hBool operator < (const hTextIterator& rhs) { return less(rhs); }
        hBool operator <= (const hTextIterator& rhs) { return lessEqual(rhs); }
        void next() {
            hChar tmputf[4];
            hUTF8::Unicode cc;
            hStringInputBuffer ib = formatter_->inputUTF8Str_[strBufIdx_];
            hUint32 adv = hUTF8::BytesInUTF8Character(ib.str_+strOs_);
            if (adv > (ib.bytes_-(strOs_+adv))) {
                if ((strBufIdx_+1) >= formatter_->inputUTF8Str_.GetSize()) {
                    value_ = 0;
                    return;
                }
                ++strBufIdx_;
                strOs_ = (strOs_+adv)-ib.bytes_;
            }
            else strOs_ += adv;

            hStringInputBuffer nib = formatter_->inputUTF8Str_[strBufIdx_];
            adv = hUTF8::BytesInUTF8Character(ib.str_+strOs_);
            if (adv > (ib.bytes_-strOs_)) {
                for (hUint32 i = strOs_, c = strOs_+adv; i < c; ++i) {
                    tmputf[i] = (strOs_+i < ib.bytes_) ? ib.str_[i] : nib.str_[i-strOs_];
                }
                hUTF8::DecodeToUnicode(tmputf, cc);
            }
            else {
                hUTF8::DecodeToUnicode(nib.str_+strOs_, cc);
                value_ = cc;
            }
            value_ = cc;
        }
        hBool equal(const hTextIterator& rhs) {
            hcAssert(formatter_ == rhs.formatter_);
            return strBufIdx_ == rhs.strBufIdx_ &&
                   strOs_ == rhs.strOs_ && 
                   formatter_ == rhs.formatter_;
        }
        hBool less(const hTextIterator& rhs) { 
            hcAssert(formatter_ == rhs.formatter_);
            return strBufIdx_ < rhs.strBufIdx_ && strOs_ < rhs.strOs_;
        }
        hBool lessEqual(const hTextIterator& rhs) { 
            hcAssert(formatter_ == rhs.formatter_);
            return strBufIdx_ <= rhs.strBufIdx_ && strOs_ <= rhs.strOs_;
        }
        hUint32 getValue() { return value_; }

    private:

        hFontFormatting* formatter_;
        hUint32 strBufIdx_;
        hUint32 strOs_;
        hUint32 value_;
    };

    struct hTextLine
    {
        hFloat  startX_;
        hFloat  startY_;
        hFloat  lineWidth_;
        hTextIterator lineStart_;
        hTextIterator lineEnd_;
    };

    friend class hTextIterator;

    typedef hVector< hTextLine > LineArrayType;
    typedef hVector< hStringInputBuffer > InputArrayType;

    static hTextLine ProcessLine(const hFontFormatting::hTextIterator& start, hFontFormatting::hTextIterator* nextStart, hFloat width, hUint32 formatFlags, hFloat scale, hFont* font);

    InputArrayType inputUTF8Str_;
    hUint32      flags_;
    hFont*       font_;
    hFloat       scale_;
    hColour      colour_;
    void*        outputPtr_;
    hUint32      outputSize_;
    hTextExtents extents_;

    LineArrayType   formattedLines_;
    hTextExtents    textExtents_;
    hUint32         primCount_;
};

}

#endif // HTEXTFORMATTING_H__