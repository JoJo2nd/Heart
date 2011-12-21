/********************************************************************

	filename: 	DeviceDX11ShaderProgram.cpp	
	
	Copyright (c) 20:12:2011 James Moran
	
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

#include "DeviceDX11ShaderProgram.h"

namespace Heart
{
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hdDX11ShaderProgram::GetShaderParameter( hUint32 i, hShaderParameter* param )
    {
        hcAssert( param );

        D3D11_SHADER_DESC desc;
        shaderInfo_->GetDesc( &desc );
        //CONSTANT BUFFERS
        for ( hUint32 buffer = 0; buffer < desc.ConstantBuffers; ++buffer )
        {
            ID3D11ShaderReflectionConstantBuffer* constInfo = shaderInfo_->GetConstantBufferByIndex( buffer );
            D3D11_SHADER_BUFFER_DESC bufInfo;
            constInfo->GetDesc( &bufInfo );

            if ( i < bufInfo.Variables )
            {
                ID3D11ShaderReflectionVariable* var = constInfo->GetVariableByIndex( i );
                D3D11_SHADER_VARIABLE_DESC varDesc;
                var->GetDesc( &varDesc );

                //lets be cheeky and nab the shader var name here!
                param->name_    = (hChar*)varDesc.Name;
                param->size_    = varDesc.Size / sizeof(hFloat);
                param->cBuffer_ = buffer;
                param->cReg_    = varDesc.StartOffset / sizeof(hFloat);

                return hTrue;
            }
            else
            {
                i -= bufInfo.Variables;
            }
        }

        return hFalse;
    }

}