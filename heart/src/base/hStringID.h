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
    /*
    *   hStringID is a unique id for a string across a single run of the program. They are 
    *   not valid across multiple runs.
    */
    class hStringID
    {
    public:
        hStringID() 
            : strEntry_(&s_default)
        {}
        hStringID(const hChar* str_id) 
            : strEntry_(get_string_id(str_id))
        {}

        hSize_t size() const { return strEntry_->byteLen_; }
        hSize_t lenght() const { return strEntry_->byteLen_; }
        const char* c_str() const { return strEntry_->strValue_; }
        hUint at(hSize_t pos) const { return strEntry_->strValue_[pos]; }
        hBool operator == (const hStringID& rhs) const { return strEntry_ == rhs.strEntry_; }
        hBool operator != (const hStringID& rhs) const { return strEntry_ != rhs.strEntry_; }
        hUint operator [] (const hSize_t pos) const { strEntry_->strValue_[pos]; }
        hUint32 hash() const { return strEntry_->strHash_; }
        hUintptr_t id() const { return (hUintptr_t)strEntry_; }
        hBool is_default() const { return strEntry_ == &s_default; }

    private:

        struct hStringIDEntry {
            hStringIDEntry() 
                : strValue_("")
                , next_(hNullptr)
                , byteLen_(0)
                , charLen_(0)
                , strHash_(0)
            {}
            hChar*          strValue_;
            hStringIDEntry* next_;
            hSize_t         byteLen_;
            hSize_t         charLen_;
            hUint32         strHash_;
        };

        static hStringIDEntry   s_default;
        static const hUint      s_hashTableBucketSize = 16*1024;
        static hUint            s_hashTableCount;
        static hStringIDEntry   s_hashTable[s_hashTableBucketSize];
        static hStringIDEntry*  get_string_id(const hChar* str);

        hStringIDEntry* strEntry_;
    };

    struct hStringIDHash
    {
        hSize_t operator () (const hStringID& rhs) { return rhs.hash(); }
    };
}

#endif // HSTRINGID_H__
