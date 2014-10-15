/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef INDEXBUFFER_H__
#define INDEXBUFFER_H__

#include "base/hTypes.h"
#include "base/hReferenceCounted.h"
#include "base/hRendererConstants.h"

namespace Heart {
namespace hRenderer {
class hIndexBuffer;

hUint getIndexCount(const hIndexBuffer* ib);
hIndexBufferType getIndexBufferType(const hIndexBuffer* ib);

}
#if 0
    class hIndexBuffer : public hdIndexBuffer,
                         public hIReferenceCounted
    {
    public:
        hFUNCTOR_TYPEDEF(void(*)(hIndexBuffer*), hZeroProc);
        hIndexBuffer(hZeroProc zeroProc) 
            : zeroProc_(zeroProc)
        {}
        ~hIndexBuffer() 
        {
        }

        hUint32			 GetIndexCount() const { return nIndices_; }
        hIndexBufferType getIndexBufferType() const { return type_; }

    private:

        
        friend class IndexBufferBuilder;

        void OnZeroRef() {
            zeroProc_(this);
        }

        hZeroProc           zeroProc_;
        hUint16*            pIndices_;//< only valid on disk based resources
        hUint32             nIndices_;
        hIndexBufferType    type_;
    };

    struct hIndexBufferMapInfo
    {
        void*           ptr_;
        hUint32         size_;
        hIndexBuffer*   ib_;
    };
#endif
}

#endif // INDEXBUFFER_H__