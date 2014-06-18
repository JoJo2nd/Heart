/********************************************************************

    filename: 	hIndexBuffer.h	
    
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

#ifndef INDEXBUFFER_H__
#define INDEXBUFFER_H__

#include "base/hTypes.h"
#include "base/hReferenceCounted.h"
#include "pal/dx11/hWin32DX11.h"

namespace Heart
{
    
namespace Cmd
{
    class FlushIndexBufferData;
}

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
}

#endif // INDEXBUFFER_H__