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
        ePTFloat,
        ePTInt,
        ePTColour,
        ePTTexture,

        ePTNone = ~0U
    };

    class HEART_DLLEXPORT hMaterialTechniquePass
    {
    public:

        hMaterialTechniquePass()
            : blendState_(hNullptr)
            , depthStencilState_(hNullptr)
            , rasterizerState_(hNullptr)
        {
        }
        hMaterialTechniquePass(hMaterialTechniquePass&& rhs)
            : blendState_(hNullptr)
            , depthStencilState_(hNullptr)
            , rasterizerState_(hNullptr)
        {
            swap(this, &rhs);
        }
        hMaterialTechniquePass& operator = (hMaterialTechniquePass&& rhs) {
            swap(this, &rhs);
            return *this;
        }
        ~hMaterialTechniquePass() {
            if (blendState_) {
                blendState_->DecRef();
                blendState_=hNullptr;
            }
            if (depthStencilState_) {
                depthStencilState_->DecRef();
                depthStencilState_=hNullptr;
            }
            if (rasterizerState_) {
                rasterizerState_->DecRef();
                rasterizerState_=hNullptr;
            }
        }

        hShaderProgram*         GetVertexShader() { return programs_[ShaderType_VERTEXPROG].weakPtr<hShaderProgram>(); }
        void                    setProgramID(hUint shadertype, hResourceID resid) { programs_[shadertype]=hResourceHandle(resid); }
        hResourceID             getProgramID(hUint shadertype) const { return programs_[shadertype].getResourceID(); }
        hUint32                 GetProgramCount() { return s_maxPrograms; }
        hShaderProgram*         getProgram(hUint32 i) { hcAssert(i < s_maxPrograms); return programs_[i].weakPtr<hShaderProgram>();}
        hBlendState*            GetBlendState() { return blendState_; }
        hDepthStencilState*     GetDepthStencilState() { return depthStencilState_; }
        hRasterizerState*       GetRasterizerState() { return rasterizerState_; }
        void                    bindBlendState(hBlendState* state) { 
            if (blendState_) {
                blendState_->DecRef();
            }
            if (state) {
                state->AddRef();
            }
            blendState_ = state; 
        }
        void                    bindDepthStencilState(hDepthStencilState* state) { 
            if (depthStencilState_) {
                depthStencilState_->DecRef();
            }
            if (state) {
                state->AddRef();
            }
            depthStencilState_ = state;
        }
        void                    bindRasterizerState(hRasterizerState* state) { 
            if (rasterizerState_) {
                rasterizerState_->DecRef();
            }
            if (state) {
                state->AddRef();
            }
            rasterizerState_ = state;
        }

        hBool   setSamplerInput(hShaderParameterID paramID, hSamplerState* srv);
        hBool   setResourceView(hShaderParameterID paramID, hShaderResourceView* view);
        hBool   setConstantBuffer(hShaderParameterID paramID, hRenderBuffer* buffer);
        hUint   getConstantBufferCount(hShaderType progtype) const { return (hUint)inputResources_[progtype].buffers_.size();}
        hUint   getSamplerCount(hShaderType progtype) const { return (hUint)inputResources_[progtype].samplerStates_.size();}
        hUint   getShaderResourceViewCount(hShaderType progtype) const { return (hUint)inputResources_[progtype].srView_.size();}
        hRenderBuffer** getConstantBuffers(hShaderType progtype) { return inputResources_[progtype].buffers_.data(); }
        hSamplerState**           getSamplers(hShaderType progtype) { return inputResources_[progtype].samplerStates_.data(); }
        hShaderResourceView**     getShaderResourceViews(hShaderType progtype) { return inputResources_[progtype].srView_.data(); }

    private:

        hMaterialTechniquePass(const hMaterialTechniquePass& rhs);
        hMaterialTechniquePass& operator = (const hMaterialTechniquePass& rhs);
        static void swap(hMaterialTechniquePass* lhs, hMaterialTechniquePass* rhs) {
            for (hUint i=0; i<ShaderType_MAX; ++i) {
                std::swap(lhs->programs_[i], rhs->programs_[i]);
            }
            std::swap(lhs->blendState_, rhs->blendState_);
            std::swap(lhs->depthStencilState_, rhs->depthStencilState_);
            std::swap(lhs->rasterizerState_, rhs->rasterizerState_);
            for (hUint i=0; i<s_maxPrograms; ++i) {
                std::swap(lhs->inputResources_[i].srView_, rhs->inputResources_[i].srView_);
                std::swap(lhs->inputResources_[i].samplerStates_, rhs->inputResources_[i].samplerStates_);
                std::swap(lhs->inputResources_[i].buffers_, rhs->inputResources_[i].buffers_);
            }
        }

        friend class hRenderer;
        friend class hMaterial;

        static const hUint32 s_maxPrograms = ShaderType_MAX;


        hResourceHandle     programs_[s_maxPrograms];
        struct 
        {
            std::vector< hShaderResourceView* >     srView_;
            std::vector< hSamplerState* >           samplerStates_;
            std::vector< hRenderBuffer* >           buffers_;
        } inputResources_[s_maxPrograms];

        /*
         * Previous should be in an array
         **/
        hBlendState*                        blendState_;
        hDepthStencilState*                 depthStencilState_;
        hRasterizerState*                   rasterizerState_;
    };

    class HEART_DLLEXPORT hMaterialTechnique
    {
    public:

        hMaterialTechnique()
            : mask_(0)
            , transparent_(false)
            , layer_(0)
        {
        }
        hMaterialTechnique(const hChar* name)
            : name_(name)
            , mask_(0)
            , transparent_(false)
            , layer_(0)
        {
        }
        hMaterialTechnique(hMaterialTechnique&& rhs) {
            swap(this, &rhs);
        }
        hMaterialTechnique& operator = (hMaterialTechnique&& rhs) {
            swap(this, &rhs);
            return *this;
        }
        ~hMaterialTechnique() {
        }

        const hChar*            getName() const { return name_.c_str(); }
        void                    SetName(const hChar* name) { name_=name; }
        hUint32                 GetMask() const { return mask_; }
        void                    SetMask(hUint32 v) { mask_=v; }
        void                    SetPasses(hUint32 count);
        hUint32                 GetPassCount() const { return (hUint)passes_.size(); }
        hMaterialTechniquePass* GetPass( hUint32 idx ) { return &passes_[idx]; }
        void                    SetSort(hBool val) { transparent_ = val; }
        hBool                   GetSort() const { return transparent_; }
        void                    SetLayer(hByte layer) { layer_ = layer; }
        hByte                   GetLayer() const { return layer_; }
        hMaterialTechniquePass* appendPass();

    private:

        hMaterialTechnique(const hMaterialTechnique& rhs);
        hMaterialTechnique& operator = (const hMaterialTechnique& rhs);
        static void swap(hMaterialTechnique* lhs, hMaterialTechnique* rhs) {
            std::swap(lhs->name_, rhs->name_);
            std::swap(lhs->passes_, rhs->passes_);
            std::swap(lhs->mask_, rhs->mask_);
            std::swap(lhs->transparent_, rhs->transparent_);
            std::swap(lhs->layer_, rhs->layer_);
        }

        friend class hRenderer;
        friend class hMaterial;

        static const hUint32 MAX_NAME_LEN = 32;
        typedef std::vector< hMaterialTechniquePass > PassArrayType;

        std::string   name_;
        PassArrayType passes_;
        hUint32       mask_;            //Set on load/create
        hBool         transparent_;
        hByte         layer_;
    };

    struct HEART_DLLEXPORT hMaterialGroup
    {
        static const hUint32 MAX_NAME_LEN = 32;
        typedef std::vector< hMaterialTechnique > TechniqueArrayType;

        hMaterialGroup()
        {
        }
        hMaterialGroup(const hChar* name) 
            : name_(name)
        {
        }
        hMaterialGroup(hMaterialGroup&& rhs) {
            swap(this, &rhs);
        }
        hMaterialGroup& operator = (hMaterialGroup&& rhs) {
            swap(this, &rhs);
            return *this;
        }
        ~hMaterialGroup() {
        }

        void setName(const hChar* name) { name_=name; }
        const hChar* getName() const { return name_.data(); }
        void  techCountHint(hUint count) { techniques_.reserve(count); }
        hUint getTechCount() const { return (hUint)techniques_.size(); }
        hMaterialTechnique* addTechnique(const hChar* name);
        hMaterialTechnique* getTech(hUint idx) { return &techniques_[idx]; }
        hMaterialTechnique* getTechniqueByName(const hChar* name);
        hMaterialTechnique* getTechniqueByMask(hUint32 mask);
        hUint getTechniqueIndexByMask(hUint32 mask);
    private:
        friend class hMaterial;

        hMaterialGroup(const hMaterialGroup& rhs);
        hMaterialGroup& operator = (const hMaterialGroup& rhs);
        static void swap(hMaterialGroup* rhs, hMaterialGroup* lhs) {
            std::swap(lhs->name_, rhs->name_);
            std::swap(lhs->techniques_, rhs->techniques_);
        }
    
        std::string        name_;
        TechniqueArrayType techniques_;
    };
}

#endif // HMATERIALTYPES_H__