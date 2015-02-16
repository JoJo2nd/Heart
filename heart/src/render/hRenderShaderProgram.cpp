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
        shaderProfiles_ = new ProgramProfile[obj_cont->shaderresources_size()];

        for (hUint i=0, n=obj_cont->shaderresources_size(); i<n; ++i) {
            auto& sres=obj_cont->shaderresources(i);
                                    /*or hRenderer::currentRenderType()*/
            if (sres.rendersystem() == proto::eShaderRenderSystem_OpenGL) { 
                hShaderProfile type = hShaderProfile::Invalid;
                switch(sres.type()) {
                case proto::eShaderType_ES2_vs       : type = hShaderProfile::ES2_vs;   break;
                case proto::eShaderType_ES2_ps       : type = hShaderProfile::ES2_ps;   break;
                case proto::eShaderType_WebGL_vs     : type = hShaderProfile::WebGL_vs; break;
                case proto::eShaderType_WebGL_ps     : type = hShaderProfile::WebGL_ps; break;
                case proto::eShaderType_ES3_vs       : type = hShaderProfile::ES3_vs;   break;
                case proto::eShaderType_ES3_ps       : type = hShaderProfile::ES3_ps;   break;
                case proto::eShaderType_FL10_vs      : type = hShaderProfile::FL10_vs;  break;
                case proto::eShaderType_FL10_ps      : type = hShaderProfile::FL10_ps;  break;
                case proto::eShaderType_FL10_gs      : type = hShaderProfile::FL10_gs;  break;
                case proto::eShaderType_FL10_cs      : type = hShaderProfile::FL10_cs;  break;
                case proto::eShaderType_FL11_vs      : type = hShaderProfile::FL11_vs;  break;
                case proto::eShaderType_FL11_ps      : type = hShaderProfile::FL11_ps;  break;
                case proto::eShaderType_FL11_gs      : type = hShaderProfile::FL11_gs;  break;
                case proto::eShaderType_FL11_cs      : type = hShaderProfile::FL11_cs;  break;
                case proto::eShaderType_FL11_hs      : type = hShaderProfile::FL11_hs;  break;
                case proto::eShaderType_FL11_ds      : type = hShaderProfile::FL11_ds;  break;
                default: break;
                }
                shaderProfiles_[i].profile_ = type;
                if (hRenderer::isProfileSupported(type)) {
                    shaderProfiles_[i].shader_ = hRenderer::compileShaderStageFromSource((hChar*)sres.source().c_str(), (hUint32)sres.source().size(), sres.entry().c_str(), type);
                }
            }
        }
        return hTrue;
    }

    hBool hShaderProgram::linkObject() {
        return hTrue;
    }
}
