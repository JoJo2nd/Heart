/********************************************************************

    filename: 	hEntityDataDefinition.h	
    
    Copyright (c) 23:1:2012 James Moran
    
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
#ifndef HENTITYDATADEFINITION_H__
#define HENTITYDATADEFINITION_H__


namespace Heart
{
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hFUNCTOR_TYPEDEF(hComponent*(*)(hEntity*), ComponentCreateCallback);
    hFUNCTOR_TYPEDEF(void (*)(hComponent*), ComponentDestroyCallback);

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class hComponentFactory : public hMapElement< hUint32, hComponentFactory >
    {
    public:

        hUint32                   componentID_;
        const hChar*              componentName_;
        hUint32                   componentPropCount_;
        const hComponentProperty* componentProperties_;
        ComponentCreateCallback   createFunc_;
        ComponentDestroyCallback  destroyFunc_;

        const hComponentProperty*       GetProperty(hUint32 hash) const
        {
            for (hUint32 i = 0; i< componentPropCount_; ++i) {
                if (componentProperties_[i].nameHash_ == hash) {
                    return componentProperties_+i;
                }
            }

            return NULL;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class hComponentDataDefinition
    {
    public:
        hComponentDataDefinition(hMemoryHeapBase* heap, hComponentFactory* base)
            : heap_(heap)
            , properties_(heap)
            , baseFactory_(base)
        {
        }
        hComponentDataDefinition(const hComponentDataDefinition& rhs)
        {
            heap_=rhs.heap_;
            dataSize_=rhs.dataSize_;
            baseFactory_=rhs.baseFactory_;
            rhs.properties_.CopyTo(&properties_);
            overrideData_=(hUint8*)hHeapMalloc(heap_, dataSize_);
            hMemCpy(overrideData_, rhs.overrideData_, dataSize_);
        }
        hComponentDataDefinition& operator = (const hComponentDataDefinition& rhs)
        {
            heap_=rhs.heap_;
            dataSize_=rhs.dataSize_;
            baseFactory_=rhs.baseFactory_;
            rhs.properties_.CopyTo(&properties_);
            overrideData_=(hUint8*)hHeapMalloc(heap_, dataSize_);
            hMemCpy(overrideData_, rhs.overrideData_, dataSize_);
            return *this;
        }
        ~hComponentDataDefinition()
        {
            hHeapFreeSafe(heap_, overrideData_);
        }

        hComponentFactory* GetComponentFactory() const { return baseFactory_; }
        void SetOverrideCount(hUint count){ properties_.Resize(count); }
        hUint32 GetOverrideCount() const { return properties_.GetSize(); }
        void* GetOverrideData(hUint idx) { return overrideData_+properties_[idx].dataOffset_; }
        hUint32 GetOverrideHash(hUint idx) { return properties_[idx].overrideHash_; }
        hUint16 GetOverrideSize(hUint idx) { return properties_[idx].size_; }
        void SetOverrideData(hUint32 propHash, void* data, hUint size)
        {
            hcAssertMsg(baseFactory_->GetProperty(propHash), "Couldn't find property hash 0x%08x", propHash);
            hUint oldsize=dataSize_;
            dataSize_+=size;
            overrideData_=(hUint8*)hHeapRealloc(heap_, overrideData_, dataSize_);
            hMemCpy(overrideData_+oldsize, data, size);
            hComponentPropertyDefault cpd;
            cpd.overrideHash_=propHash;
            cpd.dataOffset_=oldsize;
            cpd.size_=size;
            properties_.PushBack(cpd);
        }

    private:

        struct hComponentPropertyDefault
        {
            hComponentPropertyDefault()
                : overrideHash_(0)
                , dataOffset_(0)
                , size_(0)
            {

            }

            hUint32 overrideHash_;
            hUint16 dataOffset_;
            hUint16 size_;
        };

        hMemoryHeapBase*                        heap_;
        hVector< hComponentPropertyDefault >    properties_;
        hComponentFactory*                      baseFactory_;
        hUint                                   dataSize_;
        hUint8*                                 overrideData_;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class hWorldObjectTemplate : public hMapElement< hUint32, hWorldObjectTemplate >
    {
    public:
        hWorldObjectTemplate()
        {

        }
        hWorldObjectTemplate( const hWorldObjectTemplate& rhs )
        {
            entityTypeName_=rhs.entityTypeName_;
            rhs.componentDefinitions_.CopyTo(&componentDefinitions_);
        }
        hWorldObjectTemplate& operator = ( const hWorldObjectTemplate& rhs )
        {
            entityTypeName_=rhs.entityTypeName_;
            rhs.componentDefinitions_.CopyTo(&componentDefinitions_);
            return *this;
        }
        ~hWorldObjectTemplate()
        {

        }

        void                        setName(const hChar* name) { entityTypeName_ = name; }
        const hChar*                getName() const { return entityTypeName_.c_str(); }
        hUint32                     getComponentCount() const { return componentDefinitions_.GetSize(); }
        void                        appendComponentDef(const hComponentDataDefinition& compDef) { componentDefinitions_.PushBack(compDef); }
        hComponentDataDefinition*   getComponentDefinition(hUint32 idx) { return &componentDefinitions_[idx]; }

    private:

        hString                                 entityTypeName_;
        hVector< hComponentDataDefinition >     componentDefinitions_;
    };

}

#endif // HENTITYDATADEFINITION_H__