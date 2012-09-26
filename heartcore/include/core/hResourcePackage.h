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
    class HeartEngine;

    static const hUint32			HEART_RESOURCE_PATH_SIZE = 1024;

    typedef hUint64 hResourceID;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    typedef hResourceClassBase* (*OnResourceDataLoad)       (hISerialiseStream*, hIDataParameterSet*, HeartEngine*);
    typedef hBool               (*OnResourceDataCompile)    (hIDataCacheFile*, hIBuiltDataCache*, hIDataParameterSet*, HeartEngine*, hISerialiseStream*);
    typedef hBool               (*OnPackageLoadComplete)    (hResourceClassBase*, HeartEngine*);
    typedef void                (*OnResourceDataUnload)     (hResourceClassBase*, HeartEngine*);
    typedef void                (*OnPackageUnloadComplete)  (hResourceClassBase*, HeartEngine*);
    typedef hTime               (*GetTimestamp)             ();

    struct hResourceHandler : public hMapElement< hResourceType, hResourceHandler >
    {
        hResourceType           type_;
        hSharedLibAddress       loaderLib_;
        OnResourceDataLoad      binLoader_;
        OnResourceDataCompile   rawCompiler_;
        OnPackageLoadComplete   packageLink_;
        OnResourceDataUnload    resourceDataUnload_;
        OnPackageUnloadComplete packageUnlink_;
        GetTimestamp            getTimestamp_;
    };

    typedef hMap< hResourceType, hResourceHandler > hResourceHandlerMap;

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

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    struct hResourceDesc
    {
        const hChar* name_;
        const hChar* ext_;//
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class hResourcePackageV2 : public hMapElement< hUint32, hResourcePackageV2 >
    {
    public:
        hResourcePackageV2(HeartEngine* engine, hIFileSystem* filesystem, const hResourceHandlerMap* handlerMap );
        ~hResourcePackageV2();

        const hChar*            GetPackageName() const { return packageName_; }
        hUint32                 GetPackageCRC() const { return packageCRC_; }
        hResourceClassBase*     GetResource(hUint32 crc) const;

        //Only call the following on the loader thread...
        hUint32                 LoadPackageDescription(const hChar* );
        hUint32                 GetPackageDependancyCount() const;
        const hChar*            GetPackageDependancy(hUint32 i) const;
        hUint32                 GetLoadCompletionPercent() { return (loadedResources_*100) / totalResources_;  }
        hBool                   Update();//Returns true when package whiches to loaded state
        void                    Unload();
        hBool                   IsInPassiveState() const { return packageState_ == State_Ready; }
        hBool                   ToUnload() const { return packageState_ == State_Unloaded; }

    private:

        static const hUint32    MAX_PACKAGE_NAME = 128;

        typedef hVector< const hChar* > PkgLinkArray;
        typedef hMap< hUint32, hResourceClassBase > ResourceMap;

        void                        LoadResourcesState();
        hBool                       DoPostLoadLink();
        void                        DoPreUnloadUnlink();
        void                        DoUnload();

        enum State
        {
            State_Load_WaitDeps,
            State_Load_Reources,
            State_Link_Resources,
            State_Ready,
            State_Unlink_Resoruces,
            State_Unload_Resources,
            State_Unloaded,
        };

        hChar                       packageName_[MAX_PACKAGE_NAME];
        hUint32                     packageCRC_;
        State                       packageState_;
        HeartEngine*                engine_;
        const hResourceHandlerMap*  handlerMap_;
        hZipFileSystem*             zipPackage_;
        hIFileSystem*               driveFileSystem_;
        hIFileSystem*               fileSystem_;
        hUint32                     loadedResources_;
        hUint32                     totalResources_;
        hXMLDocument                descXML_;
        PkgLinkArray                links_;
        hXMLGetter                  currentResource_;
        ResourceMap                 resourceMap_;
    };

}

#endif // HRESOURCEPACKAGE_H__
