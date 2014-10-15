/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef VERTEXBUFFER_H__
#define VERTEXBUFFER_H__

namespace Heart {
namespace hRenderer {
class hVertexBuffer;
hUint getStride(const hVertexBuffer* vb);
hUint getVertexCount(const hVertexBuffer* vb);
}
#if 0
    hFUNCTOR_TYPEDEF(void(*)(class hVertexBuffer*), hVertexBufferZeroProc);
    class hVertexBuffer : public hdVtxBuffer, 
                          public hIReferenceCounted
    {
    public:
        hVertexBuffer(hVertexBufferZeroProc zeroproc)
            : zeroProc_(zeroproc)
        {}
        ~hVertexBuffer() 
        {
        }

        hUint                   getStride() const { return stride_; }
        hUint	                getVertexCount() const { return vtxCount_; }

    private:

        
        void OnZeroRef() {
            zeroProc_(this);
        }

        hVertexBufferZeroProc               zeroProc_;
        hUint32                 stride_;
        hUint32					vtxCount_;
    };

    struct hVertexBufferMapInfo
    {
        void*           ptr_;
        hUint32         size_;
        hVertexBuffer*  vb_;
    };
#endif
}

#endif // VERTEXBUFFER_H__