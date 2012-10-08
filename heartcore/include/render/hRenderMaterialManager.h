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

    struct HEARTCORE_SLIBEXPORT hRenderTechniqueInfo
    {
        hRenderTechniqueInfo() 
            : mask_(0)
        {
            hZeroMem(name_, name_.GetMaxSize());
        }

        hArray<hChar, 32>   name_;
        hUint32             mask_;
    };

    struct HEARTCORE_SLIBEXPORT hMaterialKeyContainer
    {
        hMaterial*  mat_;
        hUint32     key_; // only 19bits bit, make sure this matches hBuildRenderSortKey()

        hBool operator < (const hMaterialKeyContainer& rhs)
        {
            return key_ < rhs.key_;
        }
    };

    class HEARTCORE_SLIBEXPORT hRenderMaterialManager
    {
    public:
        hRenderMaterialManager();
        ~hRenderMaterialManager();
   

        void                        GetUniqueKey(hMaterial* mat);
        void                        RemoveKey(hMaterial* mat);
        const hRenderTechniqueInfo* AddRenderTechnique( const hChar* name );
        const hRenderTechniqueInfo* GetRenderTechniqueInfo( const hChar* name );

    private:

        typedef hVector< hRenderTechniqueInfo >     TechniqueArrayType;
        typedef hVector< hMaterialKeyContainer >    MatKeyArrayType;

        hMutex                  accessMutex_;
        hUint32                 nMatKeys_;
        hUint32                 maxKeys_;
        hMaterialKeyContainer*  matKeys_;
        TechniqueArrayType      techniques_;
    };
}

#endif // HRENDERTECHNIQUEMANAGER_H__