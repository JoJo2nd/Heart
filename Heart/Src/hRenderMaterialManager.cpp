/********************************************************************

	filename: 	hRenderTechniqueManager.cpp	
	
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

namespace Heart
{
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRenderMaterialManager::~hRenderMaterialManager()
    {
        hUint32 count = techniques_.GetSize();
        for ( hUint32 i = 0; i < count; ++i )
        {
            hDELETE(hGeneralHeap, techniques_[i].name_);
            techniques_[i].name_ = NULL;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderMaterialManager::OnMaterialLoad( hMaterial* mat, hUint32 resId )
    {
        hMutexAutoScope autoMtx( &accessMutex_ );

        hLoadedMaterial* m = loadedMaterials_.Find( resId );

        if ( !m )
        {
            hLoadedMaterial* lm = hNEW(hGeneralHeap, hLoadedMaterial);

            mat->AddRef();
            lm->material_ = mat;
            lm->instanceCount_ = 0;
            lm->firstUse_ = hTrue;

            loadedMaterials_.Insert( resId, lm );
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderMaterialManager::OnMaterialUnload( hMaterial* mat )
    {
        hMutexAutoScope autoMtx( &accessMutex_ );

        hLoadedMaterial* m = loadedMaterials_.Find( mat->GetResourceID() );
        hcAssert( m );
        hcAssert( m->instanceCount_ == 0 && m->material_->GetRefCount() == 0 );

        loadedMaterials_.Remove( m );
        delete m;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialInstance* hRenderMaterialManager::CreateMaterialInstance( hUint32 matCRC )
    {
        hMutexAutoScope autoMtx( &accessMutex_ );

        matCRC = resourceManager_->GetResourceKeyRemapping( matCRC );

        hLoadedMaterial* m = loadedMaterials_.Find( matCRC );
        hcAssert( m );
        if ( !m )
            return NULL;

        ++m->instanceCount_;
        m->firstUse_ = hFalse;
        return m->material_->CreateMaterialInstance();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderMaterialManager::DestroyMaterialInstance( hMaterialInstance* instance )
    {
        hMutexAutoScope autoMtx( &accessMutex_ );

        hLoadedMaterial* m = loadedMaterials_.Find( instance->GetParentMaterial()->GetResourceID() );
        hcAssert( m );
        --m->instanceCount_;

        m->material_->DestroyMaterialInstance( instance );

        if ( m->instanceCount_ == 0 )
        {
            m->material_->DecRef();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hRenderTechniqueInfo* hRenderMaterialManager::AddRenderTechnique( const hChar* name )
    {
        hMutexAutoScope autoMtx( &accessMutex_ );

        hUint32 count = techniques_.GetSize();
        for ( hUint32 i = 0; i < count; ++i )
        {
            if ( hStrCmp( name, techniques_[i].name_ ) == 0 )
            {
                return &techniques_[i];
            }
        }

        hcAssert( techniques_.GetSize() < 30 );
        hUint32 maskC = techniques_.GetSize();
        hRenderTechniqueInfo newInfo;
        newInfo.mask_ = 1 << maskC;
        newInfo.name_ = hNEW_ARRAY(hGeneralHeap, hChar, hStrLen(name)+1);
        hStrCopy( newInfo.name_, hStrLen(name)+1, name );

        techniques_.PushBack( newInfo );
        return &techniques_[techniques_.GetSize()-1];
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hRenderTechniqueInfo* hRenderMaterialManager::GetRenderTechniqueInfo( const hChar* name )
    {
        hMutexAutoScope autoMtx( &accessMutex_ );

        hUint32 count = techniques_.GetSize();
        for ( hUint32 i = 0; i < count; ++i )
        {
            if ( hStrCmp( name, techniques_[i].name_ ) == 0 )
            {
                return &techniques_[i];
            }
        }

        return NULL;
    }


}
