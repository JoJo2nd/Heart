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

	class hResourcePackage
	{
	public:
		hResourcePackage()
			: completedLoads_( hNEW_ALIGN(hGeneralHeap, 32, hUint32) )
		{}	
		~hResourcePackage()
		{
		    CancelPackageLoad();
            hUint32 size = resourceNames_.GetSize();
            for ( hUint32 i = 0; i < size; ++i )
            {
                hDELETE(hGeneralHeap, resourceNames_[i]);
            }

            hDELETE(hGeneralHeap, completedLoads_);
            completedLoads_ = NULL;
		}

        void                AddResourceToPackage( const hChar* resourcePath );
		void	            BeginPackageLoad( hResourceManager* resourceManager );
        hBool	            IsPackageLoaded() const;
        void                GetResourcePointers();
        hResourceClassBase* GetResource( const hChar* resourcePath ) const;
		void	            CancelPackageLoad();

	private:

        void	AddResourceToPackageInternal( const hChar* resourcePath, hResourceClassBase*& dest );
        void    BeingResourceFind();
        void    EndResourceFind();

        hVector< hChar* >               resourceNames_;
        hVector< hUint32 >              resourcecCRC_;
        hVector< hResourceClassBase* >  resourceDests_;
		hResourceManager*				resourceManager_;
        hUint32*                        completedLoads_;
	};
}

#endif // HRESOURCEPACKAGE_H__