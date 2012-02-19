/********************************************************************

	filename: 	hResourcePackage.cpp	
	
	Copyright (c) 14:8:2011 James Moran
	
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

#include "Common.h"
#include "hResourcePackage.h"
#include "hResourceManager.h"

namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::AddResourceToPackage( const hChar* resourcePath )
    {
        hUint32 crc = hResourceManager::BuildResourceCRC( resourcePath );
        hChar* path = hNEW( hGeneralHeap ) hChar[ hStrLen( resourcePath )+1 ];
        hStrCopy( path, hStrLen( resourcePath )+1, resourcePath );
        resourceNames_.PushBack( path );
        resourcecCRC_.PushBack( crc );
        resourceDests_.PushBack( (hResourceClassBase*)NULL );
    }

    //////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hResourcePackage::AddResourceToPackageInternal( const hChar* resourcePath, hResourceClassBase*& dest )
	{
        hChar* path = hNEW( hGeneralHeap ) hChar[ strlen( resourcePath )+1 ];
        strcpy( path, resourcePath );
        resourceNames_.PushBack( path );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hResourcePackage::BeginPackageLoad( hResourceManager* resourceManager )
	{
		resourceManager_ = resourceManager;
        *completedLoads_ = 0;
        //The Compiler Sucks BALLS!
        const hChar** names = (const hChar**)resourceNames_.GetBuffer();
		resourceManager_->BeginResourceLoads( names, resourceNames_.GetSize(), completedLoads_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hResourcePackage::CancelPackageLoad()
	{

	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourcePackage::IsPackageLoaded() const
    {
        hBool loaded = *completedLoads_ == resourceNames_.GetSize();
        return loaded;
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::BeingResourceFind()
    {
        resourceManager_->LockResourceDatabase();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::EndResourceFind()
    {
        resourceManager_->UnlockResourceDatabase();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::GetResourcePointers()
    {
        hcAssert( IsPackageLoaded() );
        BeingResourceFind();
        for ( hUint32 i = 0; i < *completedLoads_; ++i )
        {
            resourceDests_[i] = resourceManager_->GetResource( resourcecCRC_[i] );
        }
        EndResourceFind();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hResourcePackage::GetResource( const hChar* resourcePath ) const
    {
        for ( hUint32 i = 0; i < *completedLoads_; ++i )
        {
            if ( hStrCmp( resourcePath, resourceNames_[i] ) == 0 )
            {
                return resourceDests_[i];
            }
        }

        return NULL;
    }

}