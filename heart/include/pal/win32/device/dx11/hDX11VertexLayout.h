/********************************************************************

	filename: 	DeviceDX11VertexLayout.h	
	
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
#ifndef DEVICEDX11VERTEXLAYOUT_H__
#define DEVICEDX11VERTEXLAYOUT_H__

namespace Heart
{
    class HEART_DLLEXPORT hdDX11VertexLayout : public hMapElement< hUint32, hdDX11VertexLayout >
    {
    public:
        hdDX11VertexLayout() 
            : layout_(NULL)
        {}
        ~hdDX11VertexLayout()
        {
            if (layout_) {
                hTRACK_CUSTOM_ADDRESS_FREE("DirectX", layout_);
                layout_->Release();
            }
        }

        ID3D11InputLayout*      layout_;
    };
}

#endif // DEVICEDX11VERTEXLAYOUT_H__