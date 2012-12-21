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

namespace Heart
{
    class hMaterial;
    class hMaterialInstance;
    class hResourceManager;

    struct HEART_DLLEXPORT hRenderTechniqueInfo
    {
        hRenderTechniqueInfo() 
            : mask_(0)
        {
            hZeroMem(name_, name_.GetMaxSize());
        }

        hArray<hChar, 32>   name_;
        hUint32             mask_;
    };

    struct HEART_DLLEXPORT hMaterialKeyContainer
    {
        hMaterial*  mat_;
        hUint32     key_; // only 19bits bit, make sure this matches hBuildRenderSortKey()

        hBool operator < (const hMaterialKeyContainer& rhs)
        {
            return key_ < rhs.key_;
        }
    };

    class HEART_DLLEXPORT hRenderMaterialManager
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

        void                        SetRenderer(hRenderer* renderer) { renderer_ = renderer; }
        void                        destroyRenderResources();
        void                        GetUniqueKey(hMaterial* mat);
        void                        RemoveKey(hMaterial* mat);
        const hRenderTechniqueInfo* AddRenderTechnique( const hChar* name );
        const hRenderTechniqueInfo* GetRenderTechniqueInfo( const hChar* name );
        hdParameterConstantBlock*   GetGlobalConstantBlock(hUint32 id);
        hdParameterConstantBlock*   GetGlobalConstantBlockByAlias(const hChar* name);
        hdParameterConstantBlock*   GetGlobalConstantBlockParameterID(hShaderParameterID id);
        void                        OpenLuaMaterialLib(lua_State* L);

    private:

        typedef hVector< hRenderTechniqueInfo > TechniqueArrayType;
        typedef hVector< hMaterialKeyContainer > MatKeyArrayType;

        struct hGlobalConstantBlock : public hMapElement<hUint32, hGlobalConstantBlock>
        {
            const hChar*              name_;
            hUint32                   nameHash_; // CRC32 of block name
            hdParameterConstantBlock* constBlock_;
            hUint32                   strPoolSize_;
            hChar*                    strPool_;
            hUint32                   aliasCount_;
            const hChar**             aliases_;
            hUint32*                  aliasHashes_;
            hUint32                   paramCount_;
            hConstBlockParam*         params_;
            hUint32                   dataSize_;
            void*                     data_;
        };

        typedef hVector< hGlobalConstantBlock > ConstBlockArrayType;
        typedef hMap< hUint32, hGlobalConstantBlock > ConstBlockMapType;

        static int RegisterParameterBlock(lua_State* L);
        static int RegisterRenderTechnique(lua_State* L);

        hRenderer*              renderer_;
        hMutex                  accessMutex_;
        hUint32                 nMatKeys_;
        hUint32                 maxKeys_;
        hMaterialKeyContainer*  matKeys_;
        TechniqueArrayType      techniques_;
        ConstBlockArrayType     constBlocks_;
        ConstBlockMapType       constBlockLookUp_;
    };
}

#endif // HRENDERTECHNIQUEMANAGER_H__