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

#include "precompiled.h"
#include "texture/texture_data_store.h"
#include "tinyxml2.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

TextureDataStore::TextureDataStore() {
    freeIDs_.push_back(1);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

TextureDataStore::~TextureDataStore() {

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

boost::shared_ptr<TextureDBItem> TextureDataStore::getTextureItem(uint64 id) const {
    TextureDBConstItr it=texMap_.find(id);
    if (it != texMap_.end()) {
        return it->second;
    } else {
        return boost::shared_ptr<TextureDBItem>();
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

uint TextureDataStore::getTextureItems(const char* packageName, const char* resourceName, const char* path, TextureItemVector* outitems) const {
    if (!outitems) {
        return 0;
    }

    outitems->clear();
    for (TextureDBConstItr i=texMap_.begin(), n=texMap_.end(); i!=n; ++i) {
        if (Heart::hStrWildcardMatch(packageName, i->second->getPackageName()) &&
            Heart::hStrWildcardMatch(resourceName, i->second->getResourceName()) &&
            Heart::hStrWildcardMatch(path, i->second->getInputPath())) {
                outitems->push_back(i->second);
        }
    }

    return outitems->size();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool TextureDataStore::addTextureItem(const TextureDBItem& val) {
    TextureItemMapType::iterator itr;
    TextureDBItem texitem = val;
    texitem.setID(*freeIDs_.begin());
    freeIDs_.pop_front();
    if (freeIDs_.empty()) {
        freeIDs_.push_back(texitem.getID()+1);
    }
    itr = texMap_.find(texitem.getID());
    if (itr!=texMap_.end()) {
        return false;
    }
    boost::shared_ptr<TextureDBItem> item=boost::shared_ptr<TextureDBItem>(new TextureDBItem);
    *item=texitem;
    texMap_.insert(TextureItemMapType::value_type(texitem.getID(), item));
    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool TextureDataStore::removeTextureItem(const TextureDBItem& texitem) {
    TextureItemMapType::iterator itr;
    uint64 id=texitem.getID();
    itr=texMap_.find(id);
    if (itr==texMap_.end()) {
        return false;
    }
    texMap_.erase(itr);
    freeIDs_.push_front(id);
    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureDataStore::exportToXML(const char* exportpath, const char* packageName, const char* resourceName, const char* path) {
    tinyxml2::XMLDocument xmldoc;
    tinyxml2::XMLElement* texturesroots=xmldoc.NewElement("textures");
    xmldoc.InsertEndChild(texturesroots);
    uint texturecount=0;
    for (TextureDBConstItr i=texMap_.begin(), n=texMap_.end(); i!=n; ++i) {
        if (Heart::hStrWildcardMatch(packageName, i->second->getPackageName()) &&
        Heart::hStrWildcardMatch(resourceName, i->second->getResourceName()) &&
        Heart::hStrWildcardMatch(path, i->second->getInputPath())) {
            const TextureDBItem* dbi=i->second.get();
            tinyxml2::XMLElement* element=xmldoc.NewElement("texture");
            element->SetAttribute("inputfile", dbi->getInputPath());
            element->SetAttribute("outputfile", dbi->getOutputPath());
            element->SetAttribute("converted", dbi->getConvertAndCompress());
            element->SetAttribute("normalmapmode", dbi->getNormalMap());
            element->SetAttribute("gammacorrect", dbi->getOutputGamma() == 2.2f);
            element->SetAttribute("package", dbi->getPackageName());
            element->SetAttribute("resource", dbi->getResourceName());
            texturesroots->InsertEndChild(element);
            ++texturecount;
        }
    }
    boost::filesystem::path dirpath(exportpath);
    dirpath.remove_filename();
    boost::filesystem::create_directories(dirpath);
    xmldoc.SaveFile(exportpath);
}
