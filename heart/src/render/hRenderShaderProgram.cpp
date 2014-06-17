/********************************************************************

	filename: 	hRenderShaderProgram.cpp	
	
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
}