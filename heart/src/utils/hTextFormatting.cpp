/********************************************************************

    filename: 	hTextFormatting.cpp	
    
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

#include "utils/hTextFormatting.h"
#include "base/hProfiler.h"
#include "render/hFont.h"
#include "base/hStringUtil.h"

namespace Heart
{


    void hFontFormatting::formatText()
    {
        HEART_PROFILE_FUNC();
        //validate parameters
        hcAssert(outputPtr_);
        
        hTextLine newline;
        hTextIterator str(this);
        hTextIterator nextstr(this);
        hFloat fonth = scale_*font_->GetFontHeight();
        formattedLines_.resize(0);//Prevents releasing memory allocation
        textExtents_.width = 0;
        textExtents_.height = 0;
        while(*str) {
            newline = ProcessLine(str, &nextstr, extents_.width, flags_, scale_, font_);
            formattedLines_.push_back(newline);
            str = nextstr;
            textExtents_.height += fonth;
        }

        hFloat liney, yinc;
        if (flags_ & hFONT_ALIGN_TOP) {
            liney = extents_.height;
            yinc = -fonth;
        }
        else if (flags_ & hFONT_ALIGN_BOTTOM) {
            liney = textExtents_.height;
            yinc = -fonth;
        }
        else if (flags_ & hFONT_ALIGN_VCENTRE) {
            liney = (extents_.height - textExtents_.height)/2.f;
            yinc = -fonth;
        }
        for (hUint32 i = 0, c = formattedLines_.size(); i < c; ++i) {
            if (flags_ & hFONT_ALIGN_LEFT) {
                formattedLines_[i].startX_ = 0.f;
            }
            else if (flags_ & hFONT_ALIGN_RIGHT) {
                formattedLines_[i].startX_ = extents_.width - formattedLines_[i].lineWidth_;
            }
            else if (flags_ & hFONT_ALIGN_HCENTRE) {
                formattedLines_[i].startX_ = (extents_.width - formattedLines_[i].lineWidth_)/2.f;
            }

            formattedLines_[i].startY_ = liney;
            liney += yinc;
        }
    }

    void hFontFormatting::writeTextToBuffer(const hVec3& topleft)
    {
        HEART_PROFILE_FUNC();
        hUint32 charsWritten = 0;
        hUint32 bytesWritten = 0;
        hFloat fh = (hFloat)font_->GetFontHeight();
        hFontVex* vtx = (hFontVex*)outputPtr_;
        primCount_ = 0;

        for (hUint32 line = 0, lines = formattedLines_.size(); line < lines; ++line) {
            hTextLine* lineptr = &formattedLines_[line];
            hFloat starty = lineptr->startY_+topleft.getX();
            hFloat startx = lineptr->startX_+topleft.getY();
            for (hTextIterator c = lineptr->lineStart_; c <= lineptr->lineEnd_ && *c; ++c) {
                hUint32 cc = *c;
                const hFontCharacter& fc = *font_->GetFontCharacter(cc);

                // still got space
                if (bytesWritten+sizeof(hFontVex)*6 >= outputSize_) {
                    return;
                }

                // line break checking, gotta be ascii character for this
                if ((cc&0x7F) == cc && hIsSpace((cc&0x7F))) {
                    if (cc == ' ') {
                        startx += fc.xAdvan_*scale_;
                    }
                    else if (cc == '\t') {
                        startx += fc.xAdvan_*scale_*4;
                    }
                    continue;
                }

                if (cc && isprint(cc) == 0) continue;

                // ordered top left, top right, bottom left, bottom right
                hFloat h1 = (fc.yOffset_)*scale_;
                hFloat h2 = (fc.yOffset_+fc.height_)*scale_;
                hFloat w1 = (fc.xOffset_)*scale_;
                hFloat w2 = (fc.xOffset_+fc.width_)*scale_;
                hFontVex quad[ 4 ] = {
                    { {startx + w1, starty+h1, 0.0f}, colour_, {fc.UV1_[0], fc.UV1_[0]} },
                    { {startx + w2, starty+h1, 0.0f}, colour_, {fc.UV2_[0], fc.UV1_[0]} },
                    { {startx + w1, starty+h2, 0.0f}, colour_, {fc.UV1_[0], fc.UV2_[0]} },
                    { {startx + w2, starty+h2, 0.0f}, colour_, {fc.UV2_[0], fc.UV2_[0]} },
                };

                *vtx = quad[ 0 ]; ++vtx;
                *vtx = quad[ 1 ]; ++vtx;
                *vtx = quad[ 2 ]; ++vtx;

                *vtx = quad[ 2 ]; ++vtx;
                *vtx = quad[ 1 ]; ++vtx;
                *vtx = quad[ 3 ]; ++vtx;

                startx += fc.xAdvan_*scale_;
                ++charsWritten;

                bytesWritten += sizeof(hFontVex)*6;
                primCount_ += 2;
            }
        }
    }

    hFontFormatting::hTextLine hFontFormatting::ProcessLine(const hFontFormatting::hTextIterator& start, hFontFormatting::hTextIterator* nextStart, hFloat width, hUint32 formatFlags, hFloat scale, hFont* font)
    {
        hTextLine retLine;
        hBool seenWhiteSpace = hFalse;
        hTextIterator cur = start;
        hTextIterator end;
        hFloat lwid = 0.0f;
        hFloat spaceAdvan = font->GetFontCharacter(' ')->xAdvan_*scale;
        //TODO: handle flags?
        (void)formatFlags;
 
        for(;*cur; ++cur) {
            hUint32 cc;
            cc = *cur;

            if (isspace(cc)) {
                if ((lwid > width) || (cc == '\n')){
                    end = cur;
                    break;
                }
                else {
                    end = cur;
                }
            }

            if (cc == '\t') {
                lwid += spaceAdvan*4;
                continue;
            }

            const hFontCharacter& c = *font->GetFontCharacter(cc);
            lwid += c.xAdvan_*scale;
        }

        if (!*cur) {
            end = cur;
            *nextStart = end;
        }
        else {
            *nextStart = end;
            ++(*nextStart);
        }

        retLine.startX_ = 0;
        retLine.startY_ = 0;
        retLine.lineStart_ = start;
        retLine.lineEnd_ = end;
        retLine.lineWidth_ = lwid;

        return retLine;
    }

}
