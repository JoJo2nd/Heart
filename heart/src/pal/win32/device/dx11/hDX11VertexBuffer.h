/********************************************************************

	filename: 	DeviceDX11VertexBuffer.h	
	
	Copyright (c) 6:1:2012 James Moran
	
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
#ifndef DEVICEDX11VERTEXBUFFER_H__
#define DEVICEDX11VERTEXBUFFER_H__

namespace Heart
{
    class HEART_DLLEXPORT hdDX11VertexBuffer
    {
    public:
        hdDX11VertexBuffer() 
            : streamDescCount_(0)
        {}
        ~hdDX11VertexBuffer() {}

        hUint                   getDescCount() const { return streamDescCount_; }
        const hInputLayoutDesc* getLayoutDesc() const { return streamLayoutDesc_; }

        ID3D11Buffer*       buffer_;
        hUint32             flags_;
        hUint               stride_;
        hUint32             dataSize_;
        hUint               streamDescCount_;
        hInputLayoutDesc    streamLayoutDesc_[HEART_MAX_INPUT_STREAMS];
    };
}

#endif // DEVICEDX11VERTEXBUFFER_H__