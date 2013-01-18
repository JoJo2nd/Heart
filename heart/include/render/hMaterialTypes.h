/********************************************************************

    filename: 	hMaterialTypes.h	
    
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

#ifndef HMATERIALTYPES_H__
#define HMATERIALTYPES_H__

namespace Heart
{
    enum hParameterType
    {
        ePTFloat1,
        ePTFloat2,
        ePTFloat3,
        ePTFloat4,
        ePTFloat3x3,
        ePTFloat4x4,

        ePTNone = ~0U
    };

    class HEART_DLLEXPORT hMaterialTechniquePass
    {
    public:

        hMaterialTechniquePass()
            : vertexProgram_(NULL)
            , fragmentProgram_(NULL)
            , blendState_(NULL)
            , depthStencilState_(NULL)
            , rasterizerState_(NULL)
        {

        }

        hShaderProgram*         GetVertexShader() { return vertexProgram_; }
        void                    SetVertexShaderResID(hResourceID id) { vertexProgramID_ = id; }
        hShaderProgram*         GetFragmentShader() { return fragmentProgram_; }
        void                    SetFragmentShaderResID(hResourceID id) { fragmentProgramID_ = id; }
        hUint32                 GetProgramCount() { return s_maxPrograms; }
        hShaderProgram*         GetProgram(hUint32 i) { hcAssert(i < s_maxPrograms); return programs_[i];}
        hdBlendState*           GetBlendState() { return blendState_; }
        hdDepthStencilState*    GetDepthStencilState() { return depthStencilState_; }
        hdRasterizerState*      GetRasterizerState() { return rasterizerState_; }
        hdRenderInputObject*    GetRenderInputObject() { return &renderInput_; }
        hdRenderStreamsObject*  getRenderStreamsObject() { return &boundStreams_; }
        void                    SetBlendState(hdBlendState* state) { blendState_ = state; }
        void                    SetDepthStencilState(hdDepthStencilState* state) { depthStencilState_ = state; }
        void                    SetRasterizerState(hdRasterizerState* state) { rasterizerState_ = state; }
        hBool                   Link(hResourceManager* resManager, hRenderer* renderer, hRenderMaterialManager* matManager);
        void                    ReleaseResources(hRenderer* renderer);

        hBool   BindShaderProgram(hdShaderProgram* prog);
        hBool   BindSamplerInput(hShaderParameterID paramID, hdSamplerState* srv);
        hBool   BindResourceView(hShaderParameterID paramID, hTexture* view);
        hBool   BindConstantBuffer(hShaderParameterID paramID, hdParameterConstantBlock* buffer);
        hBool   bindInputStreams(PrimitiveType type, hIndexBuffer* idx, hVertexBuffer** vtx, hUint streamcnt);

    private:

        HEART_ALLOW_SERIALISE_FRIEND();

        friend class hRenderer;
        friend class hMaterial;

        static const hUint32 s_maxPrograms = 2;

        /*
         * Following should be in an array
         **/
        union 
        {
            hShaderProgram*     programs_[s_maxPrograms];
            struct
            {
                hShaderProgram* vertexProgram_;
                hShaderProgram* fragmentProgram_;
            };
        };
        struct
        {
            hResourceID vertexProgramID_;
            hResourceID fragmentProgramID_;
        };

        /*
         * Previous should be in an array
         **/
        hdBlendState*                       blendState_;
        hdDepthStencilState*                depthStencilState_;
        hdRasterizerState*                  rasterizerState_;
        hdRenderStreamsObject               boundStreams_;
        hdRenderInputObject                 renderInput_;
    };

    class HEART_DLLEXPORT hMaterialTechnique
    {
    public:

        hMaterialTechnique()
            : mask_(0)
        {
            hZeroMem( name_, MAX_NAME_LEN );
        }
        hMaterialTechnique&             operator = ( const hMaterialTechnique& rhs )
        {
            name_ = rhs.name_;
            transparent_ = rhs.transparent_;
            layer_ = rhs.layer_;
            mask_ = rhs.mask_;
            rhs.passes_.CopyTo( &passes_ );

            return *this;
        }
        ~hMaterialTechnique()
        {

        }

        const hChar*            GetName() const { return &name_[0]; }
        void                    SetName(const hChar* name) { hStrCopy(name_.GetBuffer(), MAX_NAME_LEN, name); }
        hUint32                 GetMask() const { return mask_; }
        void                    SetPasses(hUint32 count);
        hUint32                 GetPassCount() const { return passes_.GetSize(); }
        hMaterialTechniquePass* GetPass( hUint32 idx ) { return &passes_[idx]; }
        void                    SetSortAsTransparent(hBool val) { transparent_ = val; }
        hBool                   GetSortAsTransparent() const { return transparent_; }
        void                    SetLayer(hByte layer) { layer_ = layer; }
        hByte                   GetLayer(hByte layer) { return layer_; }
        void                    AppendPass(const hMaterialTechniquePass& pass);
        hBool                   Link(hResourceManager* resManager, hRenderer* renderer, hRenderMaterialManager* matManager);

    private:

        HEART_ALLOW_SERIALISE_FRIEND();

        friend class hRenderer;
        friend class hMaterial;
        friend class hMaterialInstance;

        static const hUint32 MAX_NAME_LEN = 32;
        typedef hVector< hMaterialTechniquePass > PassArrayType;

        hArray< hChar, MAX_NAME_LEN >   name_;
        PassArrayType                   passes_;
        hUint32                         mask_;//Set on load/create
        hBool                           transparent_;
        hByte                           layer_;
    };

    struct HEART_DLLEXPORT hMaterialGroup
    {
        static const hUint32 MAX_NAME_LEN = 32;
        typedef hVector< hMaterialTechnique > TechniqueArrayType;

        hMaterialGroup& operator = ( const hMaterialGroup& rhs )
        {
            name_ = rhs.name_;
            rhs.techniques_.CopyTo(&techniques_);
            return *this;
        }

        hArray< hChar, MAX_NAME_LEN >   name_;
        TechniqueArrayType              techniques_;
    };
}

#endif // HMATERIALTYPES_H__