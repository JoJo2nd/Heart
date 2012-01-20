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

#include "hResourceManager.h"

namespace Heart
{
    class hMaterial;
    class hMaterialInstance;
    class hResourceManager;

    struct hRenderTechniqueInfo
    {
        hRenderTechniqueInfo() 
            : name_(NULL)
            , mask_(0)
        {

        }
        hChar*          name_;
        hUint32         mask_;
    };

    class hRenderMaterialManager
    {
    public:
        hRenderMaterialManager() {}
        ~hRenderMaterialManager();
    
        void                        Initialise( hResourceManager* resourceManager ) { resourceManager_ = resourceManager; }
        void                        OnMaterialLoad( hMaterial* mat, hUint32 resId );
        void                        OnMaterialUnload( hMaterial* mat );
        hMaterialInstance*          CreateMaterialInstance( const hChar* matName ) { return CreateMaterialInstance( hResourceManager::BuildResourceCRC( matName ) ); }
        hMaterialInstance*          CreateMaterialInstance( hUint32 matCRC );
        void                        DestroyMaterialInstance( hMaterialInstance* instance );
        const hRenderTechniqueInfo* AddRenderTechnique( const hChar* name );
        const hRenderTechniqueInfo* GetRenderTechniqueInfo( const hChar* name );

    private:

        struct hLoadedMaterial : hMapElement< hUint32, hLoadedMaterial >
        {
            hMaterial*      material_;
            hUint32         instanceCount_;
            hBool           firstUse_;
        };

        typedef hVector< hRenderTechniqueInfo > TechniqueArrayType;
        typedef hMap< hUint32, hLoadedMaterial > LoadedMaterialMapType;

        hMutex                  accessMutex_;
        hResourceManager*       resourceManager_;
        LoadedMaterialMapType   loadedMaterials_;
        TechniqueArrayType      techniques_;
    };
}

#endif // HRENDERTECHNIQUEMANAGER_H__