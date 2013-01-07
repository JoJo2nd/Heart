/********************************************************************

    filename:   texture_database.cpp  
    
    Copyright (c) 4:1:2013 James Moran
    
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

#include "texture_database.h"
#include "viewer_api.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

TextureDatabase::TextureDatabase(vPackageSystem* pkgSystem)
    : pkgSystem_(pkgSystem)
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

TextureDatabase::~TextureDatabase()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

TextureDBItem* TextureDatabase::getTextureItem(const char* resourceName)
{
    TextureItemMapType::iterator itr;
    boost::crc_32_type crc;
    crc.process_bytes(resourceName, strlen(resourceName));
    
    itr=texMap_.find(crc.checksum());
    if (itr==texMap_.end()) return NULL;
    return &itr->second;// is this valid??
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureDatabase::validateAndConsolidate()
{
    std::vector< vResource* > textureRes;
    vResourceTypeID texTypeId = pkgSystem_->getTypeID("TEX");

    pkgSystem_->getResourcesOfType(texTypeId, &textureRes);

    for (TextureItemMapType::iterator i=texMap_.begin(),e=texMap_.end(); i!=e; ++i) {
        i->second.setResource(NULL);
    }

    for(size_t i=0, c=textureRes.size(); i<c; ++i) {
        vResource* itr=textureRes[i];
        TextureDBItem* texItem=getTextureItem(itr->getFullAssetName());
        if (texItem) {
            texItem->setResource(itr);
            texItem->flushToResource();
        }
        else {
            boost::crc_32_type crc;
            TextureDBItem titem;
            titem.setResource(itr);
            titem.setResourceName(itr->getFullAssetName());
            titem.setInputPath(itr->getInputFilePath());

            crc.process_bytes(itr->getFullAssetName(), strlen(itr->getFullAssetName()));
            titem.setResource(itr);
            titem.setInputPath(itr->getInputFilePath());
            texMap_[crc.checksum()] = titem;
            titem.flushToResource();
        }
    }

    ///Remove any that no-longer have a resource
    for (TextureItemMapType::iterator i=texMap_.begin(),e=texMap_.end(); i!=e; ) {
        if (i->second.getResource()==NULL){
            i=texMap_.erase(i);
        }
        else {
            ++i;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureDBItem::flushToResource()
{
    assert(resource_);
    if (resource_) {
        vResourceParameter param;
        if (convertAndCompress_) {
            resource_->setInputFilePath(outputPath_.c_str());
            if (resource_->getParameter("FORMAT")) resource_->removeParameter("FORMAT");
            if (resource_->getParameter("MIPMAPS")) resource_->removeParameter("MIPMAPS");
            param.setName("sRGB");
            param.setValue(inputGamma_>1.f || outputGamma_>1.f ? "true" : "false");
            resource_->addParameter(param);
        }
        else {
            resource_->setInputFilePath(inputPath_.c_str());
            param.setName("FORMAT");
            param.setValue("RGBA");
            resource_->addParameter(param);
            param.setName("MIPMAPS");
            param.setValue("true");
            resource_->addParameter(param);
            param.setName("sRGB");
            param.setValue(inputGamma_>1.f || outputGamma_>1.f ? "true" : "false");
            resource_->addParameter(param);
        }
    }
}
