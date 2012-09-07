/********************************************************************

	filename: 	hRenderShaderProgram.h	
	
	Copyright (c) 6:9:2012 James Moran
	
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

#ifndef HRENDERSHADERPROGRAM_H__
#define HRENDERSHADERPROGRAM_H__

namespace Heart
{
    class HEARTCORE_SLIBEXPORT hShaderProgram : public hResourceClassBase,
                                                public hPtrImpl< hdShaderProgram >
    {
    public:
        hShaderProgram() 
            : vertexInputLayoutFlags_(0)
            , shaderType_(ShaderType_MAX)
        {

        }
        ~hShaderProgram()
        {
        }

        void                    SetVertexLayout(hUint32 layout) { vertexInputLayoutFlags_ = layout; }
        hUint32                 GetVertexLayout() const { return vertexInputLayoutFlags_; }
        ShaderType              GetShaderType() const { return shaderType_; } 
        void                    SetShaderType(ShaderType shadertype) { shaderType_ = shadertype; }

        hUint32                 GetConstantBlockCount() const { return pImpl()->GetConstantBlockCount(); }
        void                    GetConstantBlockDesc(hUint32 idx, hConstantBlockDesc* desc) { pImpl()->GetConstantBlockDesc(idx, desc); }
        hUint32                 GetShaderParameterCount() { return pImpl()->GetShaderParameterCount(); }
        hBool                   GetShaderParameter(hUint32 i, hShaderParameter* param) { return pImpl()->GetShaderParameter(i, param); }


    private:

        HEART_ALLOW_SERIALISE_FRIEND();

        friend class hRenderer;

        hUint32                     vertexInputLayoutFlags_;
        ShaderType                  shaderType_;
    };
}

#endif // HRENDERSHADERPROGRAM_H__