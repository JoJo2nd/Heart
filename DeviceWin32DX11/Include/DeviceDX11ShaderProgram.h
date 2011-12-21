/********************************************************************

	filename: 	DeviceDX11ShaderProgram.h	
	
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
#ifndef DEVICEDX11SHADERPROGRAM_H__
#define DEVICEDX11SHADERPROGRAM_H__


namespace Heart
{

    class hdDX11ShaderProgram
    {
    public:
        hdDX11ShaderProgram() 
            : type_(ShaderType_MAX)
            , pixelShader_(NULL)
            , shaderInfo_(NULL)
        {

        }
        ~hdDX11ShaderProgram()
        {
            if ( shaderInfo_ )
            {
                shaderInfo_->Release();
                shaderInfo_ = NULL;
            }
        }

        hBool GetShaderParameter( hUint32 i, hShaderParameter* param );

    private:

        friend class hdDX11RenderDevice;
        friend class hdDX11RenderSubmissionCtx;

        ShaderType                  type_;
        union 
        {
            ID3D11PixelShader*      pixelShader_;
            ID3D11VertexShader*     vertexShader_;
        };
        ID3D11ShaderReflection*     shaderInfo_;
    };
}
#endif // DEVICEDX11SHADERPROGRAM_H__