/********************************************************************

    filename: 	hRenderTechniqueManager.h	
    
    Copyright (c) 11:1:2012 James Moran
    
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
#ifndef HRENDERTECHNIQUEMANAGER_H__
#define HRENDERTECHNIQUEMANAGER_H__

#include "base/hTypes.h"
#include "base/hArray.h"
extern "C" {
#   include "lua.h"
#   include "lualib.h"
#   include "lauxlib.h"
}
#include "pal/hMutex.h"
#include "base/hMemoryUtil.h"
#include "base/hRendererConstants.h"
#include "base/hMap.h"

namespace Heart
{
    class hMaterial;
    class hMaterialInstance;
    class hTexture;
    class hRenderer;
    class hRenderBuffer;

    struct hRenderTechniqueInfo
    {
        hRenderTechniqueInfo() 
            : mask_(0)
        {
            hZeroMem(name_, name_.GetMaxSize());
        }

        hArray<hChar, 32>   name_;
        hUint32             mask_;
    };

    struct hMaterialKeyContainer
    {
        hMaterial*  mat_;
        hUint32     key_; // only 19bits bit, make sure this matches hBuildRenderSortKey()

        hBool operator < (const hMaterialKeyContainer& rhs)
        {
            return key_ < rhs.key_;
        }
    };

    class hRenderMaterialManager
    {
    public:
        hRenderMaterialManager();
        ~hRenderMaterialManager();
   
        struct hConstBlockParam
        {
            const hChar* name_; // comes from string pool of hGlobalConstantBlock
            hUint32 nameHash_; // CRC32 of parameter
            hUint16 offset_;
            hUint16 size_;
        };

        void                        setRenderer(hRenderer* renderer) { renderer_ = renderer; }
        void                        createDebugMaterials();
        void                        destroyRenderResources();
        hMaterial*                  getDebugFontMat() const { return debugFontMat_; }
        hMaterial*                  getDebugPosColMat() const { return debugPosColMat_; }
        hMaterial*                  getDebugPosColUVMat() const { return debugPosColUVMat_; }
        hMaterial*                  getDebugPosColAlphaMat() const { return debugPosColAlphaMat_; }
        hMaterial*                  getDebugPosColUVAlphaMat() const { return debugPosColUVAlphaMat_; }
        void                        GetUniqueKey(hMaterial* mat);
        void                        RemoveKey(hMaterial* mat);
        const hRenderTechniqueInfo* AddRenderTechnique( const hChar* name );
        const hRenderTechniqueInfo* GetRenderTechniqueInfo( const hChar* name );
        hRenderBuffer*   GetGlobalConstantBlock(hUint32 id);
        hRenderBuffer*   GetGlobalConstantBlockByAlias(const hChar* name);
        hRenderBuffer*   GetGlobalConstantBlockParameterID(hShaderParameterID id);
        void                        registerGlobalTexture(const hChar* name, hTexture* tex, const hChar** aliases, hSize_t aliasCount, hBool takeTexture=hFalse);
        void                        resizeGlobalTexture(const hChar* name, hUint width, hUint height);
        void                        updateGlobalTexture(const hChar* name, hTexture* tex);
        hTexture*                   getGlobalTexture(const hChar* name);
        hTexture*                   getGlobalTextureByAlias(const hChar* alias);
        void                        openLuaMaterialLib(lua_State* L);

    private:

        typedef hVector< hRenderTechniqueInfo > TechniqueArrayType;
        typedef hVector< hMaterialKeyContainer > MatKeyArrayType;

        struct hGlobalConstantBlock : public hMapElement<hUint32, hGlobalConstantBlock>
        {
            const hChar*              name_;
            hUint32                   nameHash_; // CRC32 of block name
            hRenderBuffer*            constBlock_;
            hUint                     strPoolSize_;
            hChar*                    strPool_;
            hUint                     aliasCount_;
            const hChar**             aliases_;
            hUint32*                  aliasHashes_;
            hUint                     paramCount_;
            hConstBlockParam*         params_;
            hUint32                   dataSize_;
            void*                     data_;
        };

        struct hGlobalTexture : public hMapElement< hUint32, hGlobalTexture >
        {
            hTexture*     texture_;
            hUint         strPoolSize_;
            hChar*        strPool_;
            const hChar*  name_;
            hUint32       nameHash_;
            hUint         aliasCount_;
            const hChar** aliases_;
            hUint32*      aliasHashes_;
            hBool         ownsTexture_;
        };

        typedef hVector< hGlobalConstantBlock > ConstBlockArrayType;
        typedef hMap< hUint32, hGlobalConstantBlock > ConstBlockMapType;
        typedef hMap< hUint32, hGlobalTexture > GloblaTextureMapType;

        static int RegisterParameterBlock(lua_State* L);
        static int RegisterRenderTechnique(lua_State* L);
        static int registerGlobalTextureLua(lua_State* L);
        static int registerResizeLua(lua_State* L);
        static int getWindowSizeLua(lua_State* L);
        static int resizeGlobalTextureLua(lua_State* L);

        void onWindowResize(hUint width, hUint height);

        hRenderer*              renderer_;
        lua_State*              mainLuaState_;
        hMutex                  accessMutex_;
        hUint32                 nMatKeys_;
        hUint32                 maxKeys_;
        hMaterialKeyContainer*  matKeys_;
        TechniqueArrayType      techniques_;
        ConstBlockArrayType     constBlocks_;
        ConstBlockMapType       constBlockLookUp_;
        GloblaTextureMapType    globalTextures_;
        hMaterial*              debugFontMat_;
        hMaterial*              debugPosColUVMat_;
        hMaterial*              debugPosColMat_;
        hMaterial*              debugPosColAlphaMat_;
        hMaterial*              debugPosColUVAlphaMat_;
    };
}

#endif // HRENDERTECHNIQUEMANAGER_H__