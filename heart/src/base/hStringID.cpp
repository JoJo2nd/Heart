/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hStringID.h"
#include "base/hStringUtil.h"
#include "base/hMutexAutoScope.h"
#include "base/hMutex.h"

namespace Heart
{

    hStringID::hStringIDEntry hStringID::s_hashTable[hStringID::s_hashTableBucketSize];
    hUint hStringID::s_hashTableCount;

    static hMutex s_writeMutex;

    hStringID::hStringID() 
        : strEntry_(get_default_id()/*&s_default*/)
    {

    }

    hStringID::hStringID(const hChar* str_id) 
        : strEntry_(get_string_id(str_id))
    {

    }

    hStringID::hStringIDEntry* hStringID::get_default_id() {
        static hStringID::hStringIDEntry s_default;
        return &s_default;
    }

    hStringID::hStringIDEntry* hStringID::get_string_id(const hChar* str) {
        hUint len = hStrLen(str);
        if (len == 0) {
            return get_default_id();
        }
        hUint32 fullhash;
        cyMurmurHash3_x86_32(str, len, hGetMurmurHashSeed(), &fullhash);
        hUint32 key = fullhash&(s_hashTableBucketSize-1);
        hcAssert(key < s_hashTableBucketSize);
        hStringIDEntry* entry = s_hashTable+key;
        do {
            if (entry->strHash_ == fullhash && hStrCmp(str, entry->strValue_) == 0) {
                return entry;
            }
            if (entry->strHash_ == 0) {
                break;
            }
            entry = entry->next_;
        } while (entry);

        hMutexAutoScope sentry(&s_writeMutex);
        //do the search again, as between the serach and the lock, the value may have been added.
        hStringIDEntry* preventry = nullptr;
        entry = s_hashTable+key;
        do {
            if (entry->strHash_ == fullhash && hStrCmp(str, entry->strValue_) == 0) {
                return entry;
            }
            if (entry->strHash_ == 0) {
                break;
            }
            preventry = entry;
            entry = entry->next_;
        } while (entry);

        hStringIDEntry* newentry = entry ? entry : new hStringIDEntry();
        newentry->byteLen_ = len;
        newentry->charLen_ = len;
        newentry->next_ = nullptr;
        newentry->strHash_ = fullhash;
        newentry->strValue_ = new hChar[newentry->byteLen_+1];
        hStrCopy(newentry->strValue_, (hUint)newentry->byteLen_+1, str);
        if (preventry) {
            preventry->next_ = newentry;
        }
        ++s_hashTableCount;

        return newentry;
    }

    hBool hStringID::operator==(const hStringID& rhs) const
    {
        return strEntry_ == rhs.strEntry_;
    }

    hBool hStringID::operator!=(const hStringID& rhs) const
    {
        return strEntry_ != rhs.strEntry_;
    }

    hUint hStringID::operator[](const hSize_t pos) const
    {
        return strEntry_->strValue_[pos];
    }

}