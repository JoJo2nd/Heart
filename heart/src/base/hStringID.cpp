/********************************************************************

    filename:   hStringID.cpp  
    
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

namespace Heart
{

    hStringID::hStringIDEntry hStringID::s_hashTable[hStringID::s_hashTableBucketSize];
    hUint hStringID::s_hashTableCount;
    static hdMutex s_writeMutex;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hStringID::hStringIDEntry* hStringID::getStringID(const hChar* str) {
        hUint32 fullhash;
        cyMurmurHash3_x86_32(str, hStrLen(str), hGetMurmurHashSeed(), &fullhash);
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
        hStringIDEntry* preventry = hNullptr;
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

        hStringIDEntry* newentry = entry ? entry : hNEW(hStringIDEntry);
        newentry->byteLen_ = hStrLen(str);
        newentry->charLen_ = hStrLen(str);
        newentry->next_ = hNullptr;
        newentry->strHash_ = fullhash;
        newentry->strValue_ = hNEW_ARRAY(hChar, newentry->byteLen_+1);
        hStrCopy(newentry->strValue_, (hUint)newentry->byteLen_+1, str);
        if (preventry) {
            preventry->next_ = newentry;
        }
        ++s_hashTableCount;

        return newentry;
    }

}