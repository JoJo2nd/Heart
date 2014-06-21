/********************************************************************

    filename: 	hVertexBuffer.h	
    
    Copyright (c) 1:4:2012 James Moran
    
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