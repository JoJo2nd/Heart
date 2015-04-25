/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#ifndef HSTRINGID_H__
#define HSTRINGID_H__

#include "hTypes.h"
#include "cryptoMurmurHash.h"

namespace Heart 
{
    /*
    *   hStringID is a unique id for a string across a single run of the program. They are 
    *   not valid across multiple runs.
    */
    class HEART_CLASS_EXPORT hStringID
    {
    public:
        hStringID();
        explicit hStringID(const hChar* str_id);

        hFORCEINLINE hSize_t size() const { return strEntry_->byteLen_; }
        hFORCEINLINE hSize_t length() const { return strEntry_->byteLen_; }
        hFORCEINLINE const char* c_str() const { return strEntry_->strValue_; }
        hFORCEINLINE hUint at(hSize_t pos) const { return strEntry_->strValue_[pos]; }
        hBool operator == (const hStringID& rhs) const;
        hBool operator != (const hStringID& rhs) const;
        hUint operator [] (const hSize_t pos) const;
        hFORCEINLINE hUint32 hash() const { return strEntry_->strHash_; }
        hFORCEINLINE hUintptr_t id() const { return (hUintptr_t)strEntry_; }
        hFORCEINLINE hBool is_default() const { return strEntry_ == get_default_id()/*&s_default*/; }

    private:

        struct HEART_CLASS_EXPORT hStringIDEntry {
            hStringIDEntry() 
                : strValue_(nullptr)
                , next_(nullptr)
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

        static hStringIDEntry*  get_default_id();
        static const hUint      s_hashTableBucketSize = 16*1024;
        static hUint            s_hashTableCount;
        static hStringIDEntry   s_hashTable[s_hashTableBucketSize];
        static hStringIDEntry*  get_string_id(const hChar* str);

        hStringIDEntry* strEntry_;
    };
}

namespace std
{
    template<>
    struct hash<Heart::hStringID>
    {
        size_t operator () (const Heart::hStringID& rhs) const { return rhs.hash(); }
    };

    template<>
    struct less<Heart::hStringID>
    {
        bool operator () (const Heart::hStringID& lhs, const Heart::hStringID& rhs) const { return lhs.hash() < rhs.hash(); }
    };
}

#include <string>

namespace Heart {
    typedef std::string hString;
}

namespace std
{
    template<>
    struct hash < Heart::hString >
    {
        size_t operator () (const Heart::hString& rhs) const { hUint32 r; cyMurmurHash3_x86_32(rhs.c_str(), (hUint)rhs.size(), hGetMurmurHashSeed(), &r); return r; }
    };
}

#endif // HSTRINGID_H__
