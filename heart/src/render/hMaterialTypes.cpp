/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "render/hMaterialTypes.h"
#include "render/hRenderShaderProgram.h"
#include "base/hRendererConstants.h"
#include "base/hStringUtil.h"

namespace Heart
{
#if 0
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

    hShaderProgram* hMaterialTechniquePass::GetVertexShader() {
        return programs_[ShaderType_VERTEXPROG].weakPtr<hShaderProgram>();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hShaderProgram* hMaterialTechniquePass::getProgram(hUint32 i) {
        hcAssert(i < s_maxPrograms); return programs_[i].weakPtr<hShaderProgram>();
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
#endif
}