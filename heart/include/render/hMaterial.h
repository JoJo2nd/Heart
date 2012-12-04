/********************************************************************

	filename: 	hMaterial.h	
	
	Copyright (c) 1:4:2012 James Moran
	
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

#ifndef HIMATERIAL_H__
#define HIMATERIAL_H__

class MaterialEffectBuilder;
class ShaderProgramBuilder;

namespace Heart
{
	class hRenderer;
	class hRenderState;
	class hTexture;
    class hMaterialInstance;
    class hRenderSubmissionCtx;

    HEART_DLLEXPORT
    hUint32 HEART_API hFloatToFixed(hFloat input, hUint32 totalbits, hUint32 fixedpointbits);

    /*
     * See - http://realtimecollisiondetection.net/blog/?p=86/
     * MSB is our leading sort priority, LSB is last sort priority.
     * Our key is using 
     * 63 - 60 = camera / viewport / render target (0-15) - 4 bits = 64
     * 59 - 54 = sort layer (0-15) - 4 bits = 60
     * 53      = transparent flag - force transparent bits to end - 1 bits = 56
     * 52 - 29 = depth - 32 bits as 16:16 fixed point number - 32 bits = 55
     * 28 - 4  = material ID            - 19 bits = 23
     *  3 - 0  = material pass ID (0-15) - 4 bits = 4
     */
    typedef hUint64 hMaterialSortKey;

    HEART_DLLEXPORT
    hMaterialSortKey HEART_API hBuildRenderSortKey(hByte cameraID, hByte sortLayer, hBool transparent, hFloat viewSpaceDepth, hUint32 materialID, hByte pass);

    static const hUint32                    hMAX_PARAMETER_NAME_LEN = 32;
    static const hUint32                    hMAX_SEMANTIC_LEN = 32;

    struct HEART_DLLEXPORT hSamplerParameter
    {
        hSamplerParameter()
            : boundTexture_(NULL)
            , defaultTextureID_(0)
            , samplerState_(NULL)
        {
            hZeroMem(name_.GetBuffer(),name_.GetMaxSize());
        }
        ~hSamplerParameter()
        {
        }

        hArray<hChar, hMAX_PARAMETER_NAME_LEN> name_;
        hResourceID                         defaultTextureID_;
        hTexture*                           boundTexture_;
        hdSamplerState*                     samplerState_;
    };

    typedef hVector< hSamplerParameter >  hSamplerArrayType;

    typedef hUint16 hMaterialParameterID;

	class HEART_DLLEXPORT hMaterial : public hResourceClassBase
	{
	public:

		hMaterial(hMemoryHeapBase* heap) 
			: memHeap_(heap)
            , renderer_(NULL)
            , activeTechniques_(NULL)
            , manager_(NULL)
            , groups_(heap)
		{
			
		}
		~hMaterial();

        hUint32                             GetTechniqueCount() const { return activeTechniques_->GetSize(); }
        hMaterialTechnique*                 GetTechnique( hUint32 idx ) { hcAssert( activeTechniques_->GetSize() ); return &(*activeTechniques_)[idx]; }
        hMaterialTechnique*                 GetTechniqueByName( const hChar* name );
        hMaterialTechnique*                 GetTechniqueByMask( hUint32 mask );
        hRenderMaterialManager*             GetManager() const { return manager_; }
        void                                SetManager(hRenderMaterialManager* val) { manager_ = val; }
        hMaterialGroup*                     AddGroup(const hChar* name);
        void                                SetActiveGroup(const hChar* name);
        hBool                               Link(hResourceManager* resManager, hRenderer* renderer, hRenderMaterialManager* matManager);
        hUint32                             GetMatKey() const { return uniqueKey_; }
        void                                AddSamplerParameter(const hSamplerParameter& samp);
        
        /* Create Create/DestroyMaterialOverrides()*/

        /* Bind interface - return false if not set on any programs, can only set on cloned materials */
        hBool BindConstanstBuffer(hShaderParameterID id, hdParameterConstantBlock* cb);
        hBool BindTexture(hShaderParameterID id, hTexture* tex, hdSamplerState* samplerState);

        /* Allow access to parameter blocks and updating of parameters */
        hdParameterConstantBlock* GetParameterConstBlock(hShaderParameterID cbid);

	private:

        HEART_ALLOW_SERIALISE_FRIEND();

		friend class hRenderer;
        friend class hRenderMaterialManager;

       
        struct BoundTexture
        {
            hShaderParameterID paramid;
            hTexture* texture;
        };

        struct BoundConstBlock
        {
            hShaderParameterID paramid;
            hdParameterConstantBlock* constBlock;
        };

        typedef hVector< hMaterialGroup >     GroupArrayType;
        typedef hVector< hMaterialTechnique > TechniqueArrayType;

        hMaterial*                          clonedFrom_; //NULL if original, original is immutable 
        hSamplerArrayType                   defaultSamplers_;
        hMemoryHeapBase*                    memHeap_;
        hUint32                             uniqueKey_;
		hRenderer*							renderer_;
        hRenderMaterialManager*             manager_;
        GroupArrayType                      groups_;
        TechniqueArrayType*                 activeTechniques_;

        BoundConstBlock                     constBlocks_[HEART_MAX_CONSTANT_BLOCKS];
        BoundTexture                        boundTextures_[HEART_MAX_RESOURCE_INPUTS];
	};
}

#endif // HIMATERIAL_H__