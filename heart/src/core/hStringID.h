/********************************************************************

    filename:   hStringID.h  
    
    Copyright (c) 21:2:2014 James Moran
    
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

#ifndef HSTRINGID_H__
#define HSTRINGID_H__

namespace Heart 
{
    class hStringID
    {
    public:
        hStringID(const hChar* str_id);

        hSize_t
        const char* c_str() { return strValue_; }
        hBool operator == (const hStringID& rhs) { return strValue_ == rhs.strValue_; }
        hBool operator != (const hStringID& rhs) { return strValue_ != rhs.strValue_; }

    private:

        struct hStringIDEntry {
            hUint32         strHash_;
            const char*     strValue_;
            hStringIDEntry* next_;
        };

        hUint32     strHash_;
        hUint32     strLen_;
        const char* strValue_;
    };
}

#endif // HSTRINGID_H__
