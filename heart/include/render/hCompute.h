/********************************************************************

    filename:   hCompute.h  
    
    Copyright (c) 24:2:2013 James Moran
    
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

#pragma once

#ifndef HCOMPUTE_H__
#define HCOMPUTE_H__

namespace Heart
{
    class hComputeObject
    {
    public:
        hComputeObject() 
        {}
        hBool   bindShaderProgram(hShaderProgram* prog) {
            return inputs_.bindShaderProgram(prog);
        }
        hBool   bindSamplerInput(hShaderParameterID paramID, hdSamplerState* srv) {
            return inputs_.bindSamplerInput(paramID, srv);
        }
        hBool   bindResourceView(hShaderParameterID paramID, hTexture* view) {
            return inputs_.bindResourceView(paramID, view ? view->pImpl() : NULL);
        }
        hBool   bindConstantBuffer(hShaderParameterID paramID, hdParameterConstantBlock* buffer) {
            return inputs_.bindConstantBuffer(paramID, buffer);
        }
    private:
        friend class hRenderSubmissionCtx;

        hdDX11ComputeInputObject inputs_;
    };
}

#endif // HCOMPUTE_H__