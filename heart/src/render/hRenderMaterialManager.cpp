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

    hRenderMaterialManager::hRenderMaterialManager() : matKeys_(NULL)
    {
        maxKeys_ = 64;
        nMatKeys_ = 0;
        matKeys_ = (hMaterialKeyContainer*)hHeapRealloc(GetGlobalHeap(), matKeys_, sizeof(hMaterialKeyContainer)*maxKeys_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRenderMaterialManager::~hRenderMaterialManager()
    {
        hHeapFreeSafe(GetGlobalHeap(), matKeys_);
        nMatKeys_ = maxKeys_ = 0;
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
        hStrCopy( newInfo.name_, newInfo.name_.GetMaxSize(), name );

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

        return AddRenderTechnique(name);
    }

    int keyCompare(const void* lhs, const void* rhs)
    {
        return *((hMaterialKeyContainer*)lhs) < *((hMaterialKeyContainer*)rhs) ? -1 : 1;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderMaterialManager::GetUniqueKey( hMaterial* mat )
    {
        hMutexAutoScope autoMtx( &accessMutex_ );

        if (nMatKeys_+1 >= maxKeys_)
        {
            maxKeys_ *= 2;
            matKeys_ = (hMaterialKeyContainer*)hHeapRealloc(GetGlobalHeap(), matKeys_, sizeof(hMaterialKeyContainer)*maxKeys_);
        }

        //Walk the keys looking for a gap inbetween keys
        hUint32 key = 0;
        for (hUint32 i = 0; i < nMatKeys_; ++i)
        {
            if (matKeys_[i].key_ > key+1)
                break;
        }
        key += 1;
        hcAssertMsg((key&0x3FFFF)==key, "Run out of material keys, the max number of unique materials is %u", 0x3FFFF);
        matKeys_[nMatKeys_].mat_ = mat;
        matKeys_[nMatKeys_].key_ = (key&0x3FFFF);

        ++nMatKeys_;
        mat->uniqueKey_ = key;

        qsort(matKeys_, nMatKeys_, sizeof(hMaterialKeyContainer), keyCompare);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderMaterialManager::RemoveKey( hMaterial* mat )
    {
        hMutexAutoScope autoMtx( &accessMutex_ );

        //Walk the keys looking for our key
        hUint32 key = mat->GetMatKey();
        for (hUint32 i = 0; i < nMatKeys_; ++i)
        {
            if (matKeys_[i].key_ == key)
            {
                --nMatKeys_;
                matKeys_[i] = matKeys_[nMatKeys_];
#ifdef HEART_DEBUG
                matKeys_[nMatKeys_].key_ = 0;
                matKeys_[nMatKeys_].mat_ = NULL;
#endif // HEART_DEBUG
                break;
            }
        }

        --nMatKeys_;

        qsort(matKeys_, nMatKeys_, sizeof(hMaterialKeyContainer), keyCompare);
    }

}
