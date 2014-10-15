/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "render/hRenderShaderProgram.h"

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
        return hTrue;
#if 0 // !!JM old code for reference
        hBool prefersource=hTrue;
        const Heart::proto::ShaderResource* obj=nullptr;
        for (hUint i=0, n=obj_cont->shaderresources_size(); i<n; ++i) {
            if (obj_cont->shaderresources(i).rendersystem() == proto::eShaderRenderSystem_D3D11) {
                obj = &obj_cont->shaderresources(i);
            }
        }
#if defined (HEART_ALLOW_SHADER_SOURCE_COMPILE)
        class hIncluder : public hIIncludeHandler 
        {
        public:
            hIncluder() {}
            void findInclude(const hChar* includepath, const void** outdata, hUint* outlen) {
                *outdata=nullptr;
                *outlen=0;
                return;
            }

            const Heart::proto::ShaderIncludeSource* const* sections_;
            hUint sectioncount_;
        };
        hIncluder includes;
#endif
        if (!obj->has_source() || !prefersource) {
            hShaderType type;
            switch(obj->type()) {
            case proto::eShaderType_Vertex:     type=ShaderType_VERTEXPROG;    break;
            case proto::eShaderType_Pixel:      type=ShaderType_FRAGMENTPROG;  break;
            case proto::eShaderType_Geometry:   type=ShaderType_GEOMETRYPROG;  break;
            case proto::eShaderType_Hull:       type=ShaderType_HULLPROG;      break;
            case proto::eShaderType_Domain:     type=ShaderType_DOMAINPROG;    break;
            case proto::eShaderType_Compute:    type=ShaderType_COMPUTEPROG;   break;
            }
            hRenderer::get()->createShader((hChar*)obj->compiledprogram().c_str(), (hUint)obj->compiledprogram().length(), type, this);
        } else if (obj->has_source()) {
#if defined (HEART_ALLOW_SHADER_SOURCE_COMPILE)
            hShaderType type;
            switch(obj->type()) {
            case proto::eShaderType_Vertex:     type=ShaderType_VERTEXPROG;    break;
            case proto::eShaderType_Pixel:      type=ShaderType_FRAGMENTPROG;  break;
            case proto::eShaderType_Geometry:   type=ShaderType_GEOMETRYPROG;  break;
            case proto::eShaderType_Hull:       type=ShaderType_HULLPROG;      break;
            case proto::eShaderType_Domain:     type=ShaderType_DOMAINPROG;    break;
            case proto::eShaderType_Compute:    type=ShaderType_COMPUTEPROG;   break;
            }
            hShaderDefine* defines=nullptr;
            hUint definecount=0;
            hRenderer::get()->compileShaderFromSource((hChar*)obj->compiledprogram().data(), (hUint32)obj->compiledprogram().size(), obj->entry().c_str(), (hShaderProfile)obj->profile(), &includes, defines, definecount, this);
#endif
        } else {
            return hFalse;
        }
        return hTrue;
#endif
    }

    hBool hShaderProgram::linkObject() {
        return hTrue;
    }
}