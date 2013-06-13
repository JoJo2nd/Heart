/********************************************************************

    filename:   texture_database.h  
    
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

#pragma once

#ifndef TEXTURE_DATABASE_H__
#define TEXTURE_DATABASE_H__

#include <string>
#include <map>
#include <memory>

class vResource;
class vPackageSystem;

class TextureDBItem
{
public:
    TextureDBItem()
        : id_(~0ULL)
        , format_(nvtt::Format_DXT1)
        , quality_(nvtt::Quality_Normal)
        , alphaMode_(nvtt::AlphaMode_None)
        , inputGamma_(2.2f)    
        , outputGamma_(2.2f) 
        , wrapMode_(nvtt::WrapMode_Mirror)
        , mipFilter_(nvtt::MipmapFilter_Box)
        , kaiserWidth_(3.f)
        , kaiserAlpha_(4.f)
        , kaiserStretch_(1.f)
        , normalMap_(false)
        , convertAndCompress_(false)
        , sourceModified_(false)
        , enableMips_(true)
    {
    }

    void        setID(uint64 id) { id_ = id; }
    uint64      getID() const { return id_; }
    void        setInputPath(const char* inpath) { inputPath_=inpath; }
    const char* getInputPath() const { return inputPath_.c_str(); }
    void        setOutputPath(const char* outpath) { outputPath_=outpath; }
    const char* getOutputPath() const { return outputPath_.c_str(); }
    void        setPackageName(const char* package) { packageName_=package; }
    const char* getPackageName() const { return packageName_.c_str(); }
    void        setResourceName(const char* name) { resourceName_=name; }
    const char* getResourceName() const { return resourceName_.c_str(); }
    nvtt::Format    getFormat() const { return format_; }
    void            setFormat(nvtt::Format val) { format_ = val; }
    nvtt::Quality   getQuality() const { return quality_; }
    void            setQuality(nvtt::Quality val) { quality_ = val; }
    nvtt::AlphaMode getAlphaMode() const { return alphaMode_; }
    void            setAlphaMode(nvtt::AlphaMode val) { alphaMode_ = val; }
    float           getInputGamma() const { return inputGamma_; }
    void            setInputGamma(float val) { inputGamma_ = val; }
    float           getOutputGamma() const { return outputGamma_; }
    void            setOutputGamma(float val) { outputGamma_ = val; }
    nvtt::WrapMode  getWrapMode() const { return wrapMode_; }
    void            setWrapMode(nvtt::WrapMode val) { wrapMode_ = val; }
    nvtt::MipmapFilter  getMipFilter() const { return mipFilter_; }
    void                setMipFilter(nvtt::MipmapFilter val) { mipFilter_ = val; }
    float           getKaiserWidth() const { return kaiserWidth_; }
    void            setKaiserWidth(float val) { kaiserWidth_ = val; }
    float           getKaiserAlpha() const { return kaiserAlpha_; }
    void            setKaiserAlpha(float val) { kaiserAlpha_ = val; }
    float           getKaiserStretch() const { return kaiserStretch_; }
    void            setKaiserStretch(float val) { kaiserStretch_ = val; }
    bool            getNormalMap() const { return normalMap_; }
    void            setNormalMap(bool val) { normalMap_ = val; }
    bool            getConvertAndCompress() const { return convertAndCompress_; }
    void            setConvertAndCompress(bool val) { convertAndCompress_ = val; }
    bool            getSourceModified() const { return sourceModified_; }
    void            setSourceModified(bool val) { sourceModified_ = val; }
    bool            getEnableMips() const { return enableMips_; }
    void            setEnableMips(bool val) { enableMips_ = val; }

private:

    friend class boost::serialization::access;
    BOOST_SERIALIZATION_SPLIT_MEMBER();
    template<typename t_archive>
    void save(t_archive& ar, const unsigned int version) const
    {
        ar&id_;
        ar&packageName_;
        ar&inputPath_;
        ar&outputPath_;
        ar&resourceName_;
        ar&format_;
        ar&quality_;
        ar&alphaMode_;
        ar&inputGamma_;    
        ar&outputGamma_;   
        ar&wrapMode_;
        ar&mipFilter_;
        ar&kaiserWidth_;
        ar&kaiserAlpha_;
        ar&kaiserStretch_;
        ar&normalMap_;
        ar&convertAndCompress_;
        ar&sourceModified_;
        ar&enableMips_;
    }
    template<typename t_archive>
    void load(t_archive& ar, const unsigned int version)
    {
        switch(version) {
        case 1: {
            ar&id_;
            ar&packageName_;
        }
        case 0: {
            ar&inputPath_;
            ar&outputPath_;
            ar&resourceName_;
            ar&format_;
            ar&quality_;
            ar&alphaMode_;
            ar&inputGamma_;    
            ar&outputGamma_;   
            ar&wrapMode_;
            ar&mipFilter_;
            ar&kaiserWidth_;
            ar&kaiserAlpha_;
            ar&kaiserStretch_;
            ar&normalMap_;
            ar&convertAndCompress_;
            ar&sourceModified_;
            ar&enableMips_;
        }
        }
    }
    //
    //serialised
    uint64              id_;
    std::string         inputPath_;
    std::string         outputPath_;
    std::string         packageName_;
    std::string         resourceName_;
    nvtt::Format        format_;
    nvtt::Quality       quality_;
    nvtt::AlphaMode     alphaMode_;
    float               inputGamma_;    //when enabled, G is 2.2f otherwise 1.f
    float               outputGamma_;   //when enabled, G is 2.2f otherwise 1.f
    nvtt::WrapMode      wrapMode_;
    nvtt::MipmapFilter  mipFilter_;
    float               kaiserWidth_;
    float               kaiserAlpha_;
    float               kaiserStretch_;
    bool                normalMap_;
    bool                convertAndCompress_;
    bool                sourceModified_;
    bool                enableMips_;
};

BOOST_CLASS_VERSION(TextureDBItem, 1)

class TextureDataStore
{
public:
    typedef std::map<uint64, boost::shared_ptr<TextureDBItem> > TextureItemMapType;
    typedef std::vector< boost::shared_ptr<TextureDBItem> > TextureItemVector;
    typedef TextureItemMapType::const_iterator TextureDBConstItr;
    typedef TextureItemMapType::iterator TextureDBItr;

    TextureDataStore();
    ~TextureDataStore();

    boost::shared_ptr<TextureDBItem> getTextureItem(uint64 id) const;
    uint getTextureItems(const char* packageName, const char* resourceName, const char* path, TextureItemVector* outitems) const;
    TextureDBConstItr getFirstItem() const { return texMap_.begin(); }
    TextureDBItr getFirstItemMutable() { return texMap_.begin(); }
    TextureDBConstItr getEndItem() const { return texMap_.end(); }
    hBool addTextureItem(const TextureDBItem& texitem);
    hBool removeTextureItem(const TextureDBItem& texitem);
    void  exportToXML(const char* exportpath, const char* packageName, const char* resourceName, const char* path);

private:
    typedef std::list<uint64> IDVectorType;

    friend class boost::serialization::access;
    BOOST_SERIALIZATION_SPLIT_MEMBER();
    template<typename t_archive>
    void save(t_archive& ar, const unsigned int version) const
    {
        ar&freeIDs_;
        ar&texMap_;
    }
    template<typename t_archive>
    void load(t_archive& ar, const unsigned int version)
    {
        //Do last
        switch(version) {
        case 1:{
            ar&freeIDs_;
        }
        case 0: {
            ar&texMap_;
        }
        }
    }

    IDVectorType        freeIDs_;
    TextureItemMapType  texMap_;
};

BOOST_CLASS_VERSION(TextureDataStore, 1)

#endif // TEXTURE_DATABASE_H__