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

namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hResourcePackage::AddResourceToPackage(const hChar* resourcePath, hResourceManager* resourceManager)
    {
        hUint32 ret = resourceDests_.GetSize();
        resourceManager_ = resourceManager;
        hUint32 crc = hResourceManager::BuildResourceCRC(resourcePath);
        resourcecCRC_.PushBack(crc);
        resourceDests_.PushBack(NULL);

        resourceManager_->mtLoadResource(resourcePath);

        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourcePackage::IsPackageLoaded()
    {
        hBool loaded = hTrue;

        for (hUint32 i = 0, c = resourceDests_.GetSize(); i < c; ++i)
        {
            resourceDests_[i] = resourceManager_->mtGetResourceWeak(resourcecCRC_[i]);
            loaded &= resourceDests_[i] != NULL;
        }

        return loaded;
    }

}