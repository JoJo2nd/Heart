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

#define HRENDERSHADERPROGRAM_H__ //todo !!JM tmp for reference
#ifndef HRENDERSHADERPROGRAM_H__

#include "base/hTypes.h"
#include "base/hProtobuf.h"
#include "components/hObjectFactory.h"
#include "pal/dx11/hWin32DX11.h"
#include "base/hReferenceCounted.h"

namespace Heart
{
    class  hShaderProgram : public hdShaderProgram,
                                           public hIReferenceCounted
    {
    public:
        hObjectType(Heart::hShaderProgram, Heart::proto::ShaderResourceContainer);
        hFUNCTOR_TYPEDEF(void(*)(hShaderProgram*), hZeroProc);
        hShaderProgram()
            : shaderType_(ShaderType_MAX)
        {

        }
        hShaderProgram(hdRenderDevice* device, hZeroProc zeroproc) 
            : hdShaderProgram(device)
            , shaderType_(ShaderType_MAX)
            , zeroProc_(zeroproc)
        {

        }
        ~hShaderProgram()
        {
        }

        hShaderType             GetShaderType() const { return shaderType_; } 
        void                    SetShaderType(hShaderType shadertype) { shaderType_ = shadertype; }

    private:
        void OnZeroRef() {
            zeroProc_(this);
        }
        friend class hRenderer;

        hZeroProc           zeroProc_;
        hShaderType         shaderType_;
    };
}

#endif // HRENDERSHADERPROGRAM_H__