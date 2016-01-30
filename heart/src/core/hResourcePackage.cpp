/********************************************************************
 
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

#include "core/hResourcePackage.h"
#include "base/hStringUtil.h"
#include "base/hClock.h"
#include "core/hResourceManager.h"
#include "threading/hTaskGraphSystem.h"
#include "imgui.h"

namespace Heart
{
hRegisterObjectType(package, Heart::hResourcePackage, Heart::proto::PackageHeader);

hStringID load_package_description_task("heart::package::loaddescription");

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourcePackage::hResourcePackage()
        : packageState_(PkgState::Null)
        , totalResources_(0) {
        taskGraph.addTask(load_package_description_task, std::bind(&hResourcePackage::loadPackageDescription, this, std::placeholders::_1));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourcePackage::~hResourcePackage() {
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::initialise(hIFileSystem* filesystem, const hChar* packageName) {
        fileSystem_ = filesystem;
        packageName_ = hStringID(packageName);
        packageState_=PkgState::LoadPkgDesc;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    hBool hResourcePackage::serialiseObject(Heart::proto::PackageHeader*, const hSerialisedEntitiesParameters& params) const {
        hcAssertMsg(false, "Not expected to call this function");
        return hFalse;
    }

    hResourcePackage::hResourcePackage(Heart::proto::PackageHeader* ) {
        hcAssertMsg(false, "Not expected to call this function");
    }

    hBool hResourcePackage::linkObject() {
        hcAssertMsg(false, "Not expected to call this function");
        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hResourcePackage::loadPackageDescription(hTaskInfo*)
    {
        hStrCopy(packagePath_, MAX_PACKAGE_NAME, "/data/");
        hStrCat (packagePath_, MAX_PACKAGE_NAME, packageName_.c_str());
        hStrCat (packagePath_, MAX_PACKAGE_NAME, ".pkg");

        hcAssert(fileSystem_);
        //On a separate thread here so can block.

        hFileHandle pkgFileHandle_;
        auto op = fileSystem_->openFile(packagePath_, FILEMODE_READ, &pkgFileHandle_);
        auto er = fileSystem_->fileOpWait(op);
        fileSystem_->fileOpClose(op);
        hcAssertMsg(er == FileError::Ok, "Failed to open package %s", packagePath_);
        hFileStat pkgStat;
        op = fileSystem_->fstatAsync(pkgFileHandle_, &pkgStat);
        er = fileSystem_->fileOpWait(op);
        hcAssertMsg(er == FileError::Ok, "Failed to fstat package %s", packagePath_);
        pkgFileData.resize(pkgStat.filesize);
        op = fileSystem_->freadAsync(pkgFileHandle_, pkgFileData.data(), pkgFileData.size(), 0);
        er = fileSystem_->fileOpWait(op);
        hcAssertMsg(er == FileError::Ok, "Failed to read package %s", packagePath_);
        fileSystem_->closeFile(pkgFileHandle_);
        hResourceFileStream resourcefilestream(pkgFileData.data(), (hUint32)pkgFileData.size());
        google::protobuf::io::CodedInputStream resourcestream(&resourcefilestream);

        google::protobuf::uint32 headersize;
        resourcestream.ReadVarint32(&headersize);
        headersize += resourcestream.CurrentPosition();
        auto limit=resourcestream.PushLimit(headersize);
        packageHeader_.ParseFromCodedStream(&resourcestream);
        resourcestream.PopLimit(limit);

        for (hInt i=0, n=packageHeader_.packagedependencies_size(); i<n; ++i) {
            if (packageHeader_.packagedependencies(i).length()) {
                packageLinks_.push_back(hStringID(packageHeader_.packagedependencies(i).c_str()));
            }
        }
        for (hInt i=0, n=packageHeader_.entries_size(); i<n; ++i) {
            packageHeader_.mutable_entries(i)->set_entryoffset(packageHeader_.entries(i).entryoffset()+headersize);
        }
        totalResources_ = packageHeader_.entries_size();

        resourceJobArray_.resize(totalResources_);
        return 0;
    }


    void hResourcePackage::unload() {
        hcAssert(isInReadyState());//TODO: Allow canceling?
        for (const auto& i:resourceJobArray_) {
            hResourceManager::removeResource(i.resourceID_);
        }
        for (hUint i = 0, n = packageLinks_.size(); i < n; ++i) {
            hcAssert(packageLinks_[i].length());
            // It's is valid to depend on ourselves
            if (hStrCmp(packageLinks_[i].c_str(), packageName_.c_str())) {
                hResourceManager::unloadPackage(packageLinks_[i].c_str());
            }
        }
        packageState_ = PkgState::Unloaded;
    }

    hBool hResourcePackage::update() {
        hBool ret = hFalse;
        switch(packageState_) {
        case PkgState::LoadPkgDesc: {
                taskGraph.kick();
                //the resource manager will kick the queue
                packageState_=PkgState::FileReadWait;
                nextResourceToLoad_ = 0;
                linkedResources_ = 0;
            } break;
        case PkgState::FileReadWait: {
                if (taskGraph.isComplete()) {
                    //
                    packageState_=PkgState::RequestLinkedPkgs;
                }
            } break;
        case PkgState::RequestLinkedPkgs: {
                for (hUint i=0, n=packageLinks_.size(); i<n; ++i) {
                    hcAssert(packageLinks_[i].length());
                    // It's is valid to depend on ourselves
                    if (hStrCmp(packageLinks_[i].c_str(), packageName_.c_str())) {
                        hResourceManager::loadPackage(packageLinks_[i].c_str());
                    }
                }
                packageState_ = PkgState::LoadingResources;
            } break;
        case PkgState::LoadingResources: {
            hByte* file_base = (hByte*)pkgFileData.data();
            hFloat start_time = hClock::elapsed();
            for (hInt i=nextResourceToLoad_, n=packageHeader_.entries_size(); i<n && (hClock::elapsed()-start_time) < 0.01; ++i, ++nextResourceToLoad_) {
                hResourceLoadJobInputOutput* jobinfo=resourceJobArray_.data()+i;
                jobinfo->resMemStart_= file_base+packageHeader_.entries(i).entryoffset();
                jobinfo->resMemEnd_= resourceJobArray_[i].resMemStart_+packageHeader_.entries(i).entrysize();
                jobinfo->createdResource_=nullptr;
                jobinfo->resourceID_=hStringID(packageHeader_.entries(i).entryname().c_str());
                jobinfo->resourceType_=hStringID(packageHeader_.entries(i).entrytype().c_str());
                jobinfo->linked_=false;

                proto::ResourceHeader resheader;
                google::protobuf::io::ArrayInputStream resourcefilestream(jobinfo->resMemStart_, (hInt)((hPtrdiff_t)jobinfo->resMemEnd_-(hPtrdiff_t)jobinfo->resMemStart_));
                google::protobuf::io::CodedInputStream resourcestream(&resourcefilestream);

                proto::MessageContainer data_container;
                data_container.ParseFromCodedStream(&resourcestream);
                jobinfo->createdResource_ = hObjectFactory::deserialiseObject(&data_container, &jobinfo->resourceType_);
                jobinfo->objectDef_ = hObjectFactory::getObjectDefinition(jobinfo->resourceType_);
                hcAssertMsg(jobinfo->objectDef_, "Unable to locate object definition for type %s", jobinfo->resourceType_.c_str());
            }
            timer_.reset();
            if (nextResourceToLoad_ == packageHeader_.entries_size()) {
                pkgFileData.clear();
                packageState_=PkgState::ResourceLinking;
            }
        } break;
        case PkgState::ResourceLinking:{
            hUint32 linked = 0;
            for (auto& res : resourceJobArray_) {
                if (!res.linked_ && res.objectDef_->link_(res.createdResource_)) {
                    res.linked_ = true;
                    hResourceManager::addResource(res.createdResource_, packageName_, res.resourceID_, res.resourceType_, res.objectDef_->destroy_);
                }
                if (res.linked_) {
                    ++linked;
                }
            }
            packageState_ = linked == resourceJobArray_.size() ? PkgState::Loaded : PkgState::ResourceLinking;
        } break;
        case PkgState::Destroying: {
            for (auto& res : resourceJobArray_) {
                res.objectDef_->destroy_(res.createdResource_);
                res.createdResource_ = nullptr;
            }
            resourceJobArray_.clear();
            packageState_ = PkgState::Destroyed;
        }
        case PkgState::Loaded: {

        } break;
        default:{
        } break;
        }

        return ret;
    }

#if HEART_DEBUG_INFO
    void hResourcePackage::printResourceInfo() {
        ImGui::Text("Package state: %s", getPackageStateString());
        for (const auto& res : resourceJobArray_) {
            ImGui::Text("Resource: %s Type: %s Linked: %s", res.resourceID_.c_str(), res.objectDef_->objectName_.c_str(), res.linked_ ? "yes" : "no");
        }
    }

    const hChar* hResourcePackage::getPackageStateString() const
    {
        static const hChar* stateStr[] = {
            "Null",
            "LoadPkgDesc",
            "FileReadWait",
            "RequestLinkedPkgs",
            "LoadingResources",
            "ResourceLinking",
            "Loaded",
            "Unloaded",
            "Destroying",
            "Destroyed",
        };
        return stateStr[(hUint32)packageState_];
    }
#endif
}