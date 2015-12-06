/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"

#define HEART_VERIFY_FREELIST (1)

namespace Heart {
template<typename t_ty, hUintptr_t pageSize = 4096>
class hFreeListAllocator {
    static const hSize_t NodePadSize = sizeof(t_ty) - sizeof(void*) - sizeof(hUint);
    static const hUintptr_t PtrMask = ~(pageSize - 1);
    static const hUint EmptyBlockCount = pageSize / sizeof(t_ty);
    static_assert(pageSize >= sizeof(t_ty), "pageSize is smaller than type size");
    static_assert((pageSize & (pageSize - 1)) == 0, "pageSize must be a power of 2");

    struct FreelistNode {
        FreelistNode* next;
        hUint         nodeCount; // number of nodes after this that are free for allocation, inclusive of this node too.
        hByte         pad[NodePadSize];
    };
    struct FreelistPage {
        std::unique_ptr<hByte> pageMem;
        hUint         freeCount = EmptyBlockCount;
        FreelistNode* freelist = nullptr;
    };

    std::vector<FreelistPage> freelistPages;

    FreelistPage* getPage() {
        for (hSize_t i = 0, n = freelistPages.size(); i < n; ++i) {
            if (freelistPages[i].freeCount) {
                hUint j = 0;
                while (i - j > 0 && (freelistPages[i].freeCount-1) < freelistPages[i - j].freeCount) ++j;
                if (j) std::swap(freelistPages[i], freelistPages[i - j]);
                return &freelistPages[i];
            }
        }
        FreelistPage new_page;
        new_page.pageMem.reset(new hByte[pageSize]);
        new_page.freelist = (FreelistNode*)new_page.pageMem.get();
        new_page.freelist->nodeCount = new_page.freeCount;
        new_page.freelist->next = nullptr;
        freelistPages.insert(freelistPages.begin(), std::move(new_page));
        return &freelistPages[0];
    }
    FreelistPage* findPage(void* ptr) {
        for (hSize_t i = 0, n = freelistPages.size(); i < n; ++i) {
            if (ptr >= freelistPages[i].pageMem.get() && ptr < (freelistPages[i].pageMem.get()+pageSize)) {
                hUint j = 0;
                while (i+j < (n-1) && (freelistPages[i].freeCount+1) > freelistPages[i+j+1].freeCount) ++j;
                if (j) std::swap(freelistPages[i], freelistPages[i+j]);
                return &freelistPages[i+j];
            }
        }
        hcAssertFailMsg("Unable to find page for freelist node ptr 0x%p", ptr);
        return nullptr;
    }
public:

    t_ty* allocate() {
        auto* page = getPage();
        --page->freeCount;
        auto* ptr = (t_ty*)page->freelist;
        --page->freelist->nodeCount;
        if (page->freelist->nodeCount == 0) {
            page->freelist = page->freelist->next;
        }
        else {
            (page->freelist+1)->nodeCount = page->freelist->nodeCount;
            (page->freelist + 1)->next = page->freelist->next;
            page->freelist = page->freelist + 1;
        }

#if HEART_VERIFY_FREELIST
        for (FreelistNode* i = page->freelist; i; i = i->next) {
            hcAssertMsg(i < i->next || i->next == nullptr, "Freelist allocator unallocated list is not address sorted");
            hcAssertMsg(i + i->nodeCount != i->next || i->next == nullptr, "Freelist allocator unallocated list is not merged correctly");
        }
        for (hSize_t i = 0, n = freelistPages.size() - 1; i < n; ++i) {
            hcAssertMsg(freelistPages[i].freeCount >= freelistPages[i + 1].freeCount, "Freelist bin pages aren't sorted correctly");
        }
#endif

        return ptr;
    }
    void release(void* ptr) {
        if (!ptr) return;
        // insertion sort pointer back in, merging into freeblock, if required.
        auto* page = findPage(ptr);
        hcAssert(ptr >= page->pageMem.get() && ptr < page->pageMem.get()+pageSize);
        ++page->freeCount;
        auto* fl_new_ptr = (FreelistNode*)ptr;
        if (!page->freelist) {
            page->freelist = fl_new_ptr;
            fl_new_ptr->nodeCount = 1;
            fl_new_ptr->next = nullptr;
            return;
        }
        
        auto* prev = (FreelistNode*)nullptr;
        auto* node = page->freelist;
        while(node) {
            if (fl_new_ptr > node) {
                if (prev) prev->next = fl_new_ptr;
                fl_new_ptr->next = node;
                fl_new_ptr->nodeCount = 1;

                //check for merge forward
                if (fl_new_ptr+1 == fl_new_ptr->next) {
                    if (prev) prev->next = fl_new_ptr;
                    fl_new_ptr->nodeCount += fl_new_ptr->next->nodeCount;
                    fl_new_ptr->next = fl_new_ptr->next;
                }
                //check for merge backward
                if (prev && prev+prev->nodeCount == fl_new_ptr) {
                    prev->nodeCount += fl_new_ptr->nodeCount;
                    prev->next = fl_new_ptr->next;
                }
                break;
            }
            prev = node;
            node = node->next;
        }

#if HEART_VERIFY_FREELIST
        for (FreelistNode* i=page->freelist; i; i=i->next) {
            hcAssertMsg(i < i->next || i->next == nullptr, "Freelist allocator unallocated list is not address sorted");
            hcAssertMsg(i+i->nodeCount != i->next || i->next == nullptr, "Freelist allocator unallocated list is not merged correctly");
        }
        for (hSize_t i=0, n=freelistPages.size()-1; i<n; ++i) {
            hcAssertMsg(freelistPages[i].freeCount <= freelistPages[i+1].freeCount, "Freelist bin pages aren't sorted correctly");
        }
#endif
        if (page->freeCount == EmptyBlockCount) {
            freelistPages.erase(freelistPages.begin() + (page - freelistPages.data()));
        }
    }
};
}