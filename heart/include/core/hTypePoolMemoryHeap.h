/********************************************************************

    filename:   hTypePoolMemoryHeap.h  
    
    Copyright (c) 13:1:2013 James Moran
    
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

#ifndef HTYPEPOOLMEMORYHEAP_H__
#define HTYPEPOOLMEMORYHEAP_H__

namespace Heart
{
    template< typename t_ty, hUint t_blockSize >
    class hTypePoolMemoryHeap : public hMemoryHeapBase
    {
        HEART_MEMORY_HEAP_CLASS;
    public:

        hTypePoolMemoryHeap(hMemoryHeapBase* base) 
            : hMemoryHeapBase("TypePoolHeap")
            , baseHeap_(base)
            , alloced_(0)
            , poolBlocks_(base)
            , freeList_(NULL)
        {
            freeList_.SetAutoDelete(hFalse);
        }

        ~hTypePoolMemoryHeap()
        {

        }
        void                        create(hUint32 /*sizeInBytes*/, hBool /*threadLocal*/) 
        {
        }
        void		                destroy() {
            hUint leftAllocs=0;
            for (hUint i=0,c=poolBlocks_.GetSize(); i<c; ++i) {
                leftAllocs+=poolBlocks_[i].alloced_;
                releasePoolBlock(&poolBlocks_[i]);
            }
        }
        void*		                alloc(hUint32 size, hUint32 alignment) {
            hcAssert(size==sizeof(t_ty));
            void* ptr=allocInternal();
            hMH_TRACK_ALLOC_UNKNOWN(ptr, s_typeSize, 0);
            return ptr;
        }
        void*		                alloc(hUint32 size, hUint32 alignment, const hChar* file, hUint32 line) {
            hcAssert(size==sizeof(t_ty));
            void* ptr=allocInternal();
            hMH_TRACK_ALLOC(ptr, file, line, s_typeSize, 0);
            return ptr;
        }
        void*		                realloc(void* ptr, hUint32 size) {
            hcAssert(ptr==NULL && size==sizeof(t_ty));
            void* block=allocInternal();
            hMH_TRACK_ALLOC_UNKNOWN(ptr, s_typeSize, 0);
            return block;
        }
        void*		                realloc(void* ptr, hUint32 size, const hChar* file, hUint32 line) {
            hcAssert(ptr==NULL && size==sizeof(t_ty));
            void* block=allocInternal();
            hMH_TRACK_ALLOC(ptr, file, line, s_typeSize, 0);
            return block;
        }
        void		                release(void* ptr) {
            if (!ptr) return;
            hcAssert(pointerBelongsToMe(ptr));

            freeList_.PushBack((hFreeLink*)ptr);

            for (hUint i=0,c=poolBlocks_.GetSize(); i<c; ++i) {
                if (ptr >= poolBlocks_[i].mem_ && ptr < ((hUint8*)poolBlocks_[i].mem_+s_poolBlockSize)) {
                    --poolBlocks_[i].alloced_;
                    if (poolBlocks_[i].alloced_==0) {
                        releasePoolBlock(&poolBlocks_[i]);
                    }
                }
            }

            hMH_RELEASE_TRACK_INFO(ptr, s_typeSize);
        }
        hMemoryHeapBase::HeapInfo	usage() {
            return hMemoryHeapBase::HeapInfo();
        }
        hUint32                     totalAllocationCount() const { return alloced_; }
        hBool		                pointerBelongsToMe(void* ptr) {
            for (hUint i=0,c=poolBlocks_.GetSize(); i<c; ++i) {
                if (ptr >= poolBlocks_[i].mem_ && ptr < ((hUint8*)poolBlocks_[i].mem_+s_poolBlockSize)) {
                    return true;
                }
            }

            return false;
        }

    private:


        hTypePoolMemoryHeap( const hMemoryHeap& c );

        struct hFreeLink : public hLinkedListElement< hFreeLink >
        {
        };

        struct hTypePoolBlock
        {
            hUint           alloced_;
            void*           mem_;
        };

        static const hUint s_poolBlockSize=(sizeof(t_ty)*t_blockSize);
        static const hUint s_typeSize=hMax(sizeof(hFreeLink),sizeof(t_ty));

        typedef hVector< hTypePoolBlock > PoolBlockVectorType;
        typedef hLinkedList< hFreeLink > FreeListType;

        void* allocInternal() {
            hMH_PRE_ACTION();
            if (freeList_.GetSize() == 0) {
                createPoolBlock();
            }
            hFreeLink* block=freeList_.GetHead();
            freeList_.Remove(block);
            for (hUint i=0,c=poolBlocks_.GetSize(); i<c; ++i) {
                if ((hUint8*)block >= poolBlocks_[i].mem_ && (hUint8*)block < ((hUint8*)poolBlocks_[i].mem_+s_poolBlockSize)) {
                    ++poolBlocks_[i].alloced_;
                }
            }
            hMH_POST_ACTION();
            return block;
        }
        void createPoolBlock() {
            hTypePoolBlock* poolblock=NULL;
            for (hUint i=0,c=poolBlocks_.GetSize(); i<c; ++i) {
                if (!poolBlocks_[i].mem_) {
                    // got a previous that has been released
                    poolblock=&poolBlocks_[i];
                    break;
                }
            }

            if (!poolblock) {
                poolBlocks_.Resize(poolBlocks_.GetSize()+1);
                poolblock=&poolBlocks_[poolBlocks_.GetSize()-1];
            }

            hUint8* ptr=hNEW_ARRAY(baseHeap_, hUint8, s_poolBlockSize);
            poolblock->mem_=(void*)ptr;
            poolblock->alloced_=0;
            for (hUint ele=0; ele<t_blockSize; ++ele) {
                freeList_.PushBack((hFreeLink*)ptr);
                ptr+=s_typeSize;
            }
        }
        void releasePoolBlock(hTypePoolBlock* block) {
            hcAssert(block->alloced_==0);
            for (hFreeLink* i=freeList_.GetHead(); i; ) {
                if ((void*)i >= block->mem_ && (void*)i < ((hUint8*)block->mem_+s_poolBlockSize)) {
                    hFreeLink* del=i;
                    i=i->GetNext();
                    freeList_.Remove(del);
                }
                else {
                    i=i->GetNext();
                }
            }

            block->alloced_=0;
            hHeapFreeSafe(baseHeap_, block->mem_);
        }

        hMemoryHeapBase*    baseHeap_;
        hUint32				alloced_;
        PoolBlockVectorType poolBlocks_;
        FreeListType        freeList_;
    };
}

#endif // HTYPEPOOLMEMORYHEAP_H__