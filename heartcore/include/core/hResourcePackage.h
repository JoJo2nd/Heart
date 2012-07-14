/********************************************************************

	filename: 	hResourcePackage.h	
	
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
#ifndef HRESOURCEPACKAGE_H__
#define HRESOURCEPACKAGE_H__

namespace Heart
{
	class hResourceManager;
	class hResourceClassBase;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class HEARTCORE_SLIBEXPORT hResourcePackage
	{
	public:
		hResourcePackage()
            : resourceManager_(NULL)
		{
        }	
		~hResourcePackage()
		{
		}

        hUint32             AddResourceToPackage(const hChar* resourcePath, hResourceManager* resourceManager);
        hBool               IsPackageLoaded();
        hUint32             GetPackageSize() const { return resourceDests_.GetSize(); }
        hResourceClassBase* GetResource(hUint32 id) { hcAssert(IsPackageLoaded()); return resourceDests_[id]; }

    private:

        hVector< hUint32 >              resourcecCRC_;
        hVector< hResourceClassBase* >  resourceDests_;
		hResourceManager*				resourceManager_;
	};
}

#endif // HRESOURCEPACKAGE_H__