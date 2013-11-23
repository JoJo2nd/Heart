/********************************************************************

    filename: 	hMaterialTypes.cpp	
    
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

namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialTechniquePass::setConstantBuffer(hShaderParameterID paramID, hRenderBuffer* buffer)
    {
        hBool succ=false;
        for (hUint i=0, n=s_maxPrograms; i<n; ++i) {
            hShaderProgram* prog=programs_[i].weakPtr<hShaderProgram>();
            if (prog) {
                hUint32 idx = prog->GetInputRegister(paramID);
                if (idx > HEART_MAX_RESOURCE_INPUTS) {
                    continue;
                }
                succ=true;
                if (idx+1>inputResources_[i].buffers_.size()) {
                    hUint oldsize=(hUint)inputResources_[i].buffers_.size();
                    inputResources_[i].buffers_.resize(idx+1);
                    for (hUint b=oldsize; b<idx+1; ++b) {
                        inputResources_[i].buffers_[b]=hNullptr;
                    }
                }
                inputResources_[i].buffers_[idx]=buffer;
            }
        }
        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialTechniquePass::setSamplerInput(hShaderParameterID paramID, hSamplerState* srv)
    {
        hBool succ=false;
        for (hUint i=0, n=s_maxPrograms; i<n; ++i) {
            hShaderProgram* prog=programs_[i].weakPtr<hShaderProgram>();
            if (prog) {
                hUint32 idx = prog->GetInputRegister(paramID);
                if (idx > HEART_MAX_RESOURCE_INPUTS) {
                    continue;
                }
                succ=true;
                if (idx+1>inputResources_[i].samplerStates_.size()) {
                    hUint oldsize=(hUint)inputResources_[i].samplerStates_.size();
                    inputResources_[i].samplerStates_.resize(idx+1);
                    for (hUint b=oldsize; b<idx+1; ++b) {
                        inputResources_[i].samplerStates_[b]=hNullptr;
                    }
                }
                inputResources_[i].samplerStates_[idx]=srv;
            }
        }
        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialTechniquePass::setResourceView(hShaderParameterID paramID, hShaderResourceView* view)
    {
        hBool succ=false;
        for (hUint i=0, n=s_maxPrograms; i<n; ++i) {
            hShaderProgram* prog=programs_[i].weakPtr<hShaderProgram>();
            if (prog) {
                hUint32 idx = prog->GetInputRegister(paramID);
                if (idx > HEART_MAX_RESOURCE_INPUTS) {
                    continue;
                }
                succ=true;
                if (idx+1>inputResources_[i].srView_.size()) {
                    hUint oldsize=(hUint)inputResources_[i].srView_.size();
                    inputResources_[i].srView_.resize(idx+1);
                    for (hUint b=oldsize; b<idx+1; ++b) {
                        inputResources_[i].srView_[b]=hNullptr;
                    }
                }
                inputResources_[i].srView_[idx]=view;
            }
        }
        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialTechniquePass* hMaterialTechnique::appendPass()
    {
        passes_.push_back(hMaterialTechniquePass());
        return &(*passes_.rbegin());
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterialTechnique::SetPasses(hUint32 count)
    {
        passes_.reserve(count);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialTechnique* hMaterialGroup::getTechniqueByName(const hChar* name) {
        for (hUint i=0,n=(hUint)techniques_.size(); i<n; ++i) {
            if (hStrICmp(name, techniques_[i].getName()) == 0) {
                return &techniques_[i];
            }
        }
        return hNullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialTechnique* hMaterialGroup::getTechniqueByMask(hUint32 mask) {
        for (hUint i=0,n=(hUint)techniques_.size(); i<n; ++i) {
            if (techniques_[i].GetMask()==mask) {
                return &techniques_[i];
            }
        }
        return hNullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hMaterialGroup::getTechniqueIndexByMask(hUint32 mask) {
        for (hUint i=0,n=(hUint)techniques_.size(); i<n; ++i) {
            if (techniques_[i].GetMask()==mask) {
                return i;
            }
        }
        return hErrorCode;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialTechnique* hMaterialGroup::addTechnique(const hChar* name) {
        for (auto i=techniques_.begin(),n=techniques_.end(); i!=n; ++i) {
            if (hStrCmp(i->getName(), name)==0) {
                //overwrite technique that matches this name
                i->SetName(name);
                return &(*i);
            }
        }
        techniques_.push_back(name);
        return &(*techniques_.rbegin());
    }

}