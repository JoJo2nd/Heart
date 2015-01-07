/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "render/hRenderShaderProgram.h"
#include "base/hRendererConstants.h"
#include "render/hRenderer.h"

namespace Heart
{
    hRegisterObjectType(shader, Heart::hShaderProgram, Heart::proto::ShaderResourceContainer);

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    hBool hShaderProgram::serialiseObject(Heart::proto::ShaderResourceContainer* obj) const {
        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    hBool hShaderProgram::deserialiseObject(Heart::proto::ShaderResourceContainer* obj_cont) {
        for (hUint i=0, n=obj_cont->shaderresources_size(); i<n; ++i) {
            auto& sres=obj_cont->shaderresources(i);
                                    /*or hRenderer::currentRenderType()*/
            if (sres.rendersystem() == proto::eShaderRenderSystem_OpenGL) { 
                hShaderProfile type = eShaderProfile_vs4_0;
                switch(sres.type()) {
                case proto::eShaderType_vs4_0       : type = eShaderProfile_vs4_0; break; 
                case proto::eShaderType_vs4_1       : type = eShaderProfile_vs4_1; break; 
                case proto::eShaderType_vs5_0       : type = eShaderProfile_vs5_0; break; 
                case proto::eShaderType_ps4_0       : type = eShaderProfile_ps4_0; break; 
                case proto::eShaderType_ps4_1       : type = eShaderProfile_ps4_1; break; 
                case proto::eShaderType_ps5_0       : type = eShaderProfile_ps5_0; break; 
                case proto::eShaderType_gs4_0       : type = eShaderProfile_gs4_0; break; 
                case proto::eShaderType_gs4_1       : type = eShaderProfile_gs4_1; break; 
                case proto::eShaderType_gs5_0       : type = eShaderProfile_gs5_0; break; 
                case proto::eShaderType_cs4_0       : type = eShaderProfile_cs4_0; break; 
                case proto::eShaderType_cs4_1       : type = eShaderProfile_cs4_1; break; 
                case proto::eShaderType_cs5_0       : type = eShaderProfile_cs5_0; break; 
                case proto::eShaderType_hs5_0       : type = eShaderProfile_hs5_0; break; 
                case proto::eShaderType_ds5_0       : type = eShaderProfile_ds5_0; break; 
                default: break;
                }
                shader_ = hRenderer::compileShaderStageFromSource((hChar*)sres.source().c_str(), (hUint32)sres.source().size(), sres.entry().c_str(), type);
            }
        }
        return hTrue;
    }

    hBool hShaderProgram::linkObject() {
        return hTrue;
    }
}
