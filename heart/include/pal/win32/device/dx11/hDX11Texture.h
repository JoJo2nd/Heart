/********************************************************************

	filename: 	DeviceDX11Texture.h	
	
	Copyright (c) 18:12:2011 James Moran
	
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
    class HEART_DLLEXPORT hdDX11Texture
    {
    public:
        hdDX11Texture() 
            : dx11Texture_(NULL)
            , renderTargetView_(NULL)
            , depthStencilView_(NULL)
            , shaderResourceView_(NULL)
        {}
        ~hdDX11Texture() 
        {}

    private:

        friend class hdDX11RenderDevice;
        friend class hdDX11RenderSubmissionCtx;
        friend class hdDX11ComputeInputObject;
        friend class hdDX11RenderInputObject;

        ID3D11Texture2D*            dx11Texture_;
        ID3D11RenderTargetView*     renderTargetView_;
        ID3D11DepthStencilView*     depthStencilView_;
        ID3D11ShaderResourceView*   shaderResourceView_;

    };
}