/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

namespace Heart
{
#if defined (HEART_PLAT_WINDOWS)
#   pragma message ("STUBBED OUT: hEntity")
#elif defined (HEART_PLAT_LINUX)
#   message ("STUBBED OUT: hEntity")
#endif
#if 0
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hComponent* hEntity::FindComponentByID(hUint32 id)
    {
        for (hUint32 i = 0; i < components_.size(); ++i)
        {
            if(components_[i].componentHash_ == id)
                return components_[i].component_;
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hEntity::AddComponent(hComponent* component)
    {
        hcPrintf("Stub :"__FUNCTION__);
//         hcAssert(FindComponentByID(component->GetID()) == NULL);
// 
//         hComponentContainer c;
//         c.component_ = component;
//         c.componentHash_ = component->GetID();
//         components_.PushBack(c);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hEntity::RemoveComponent( hComponent* component )
    {
        for (hUint32 i = 0; i < components_.size(); ++i)
        {
            if(components_[i].component_ == component)
                components_[i].component_ = NULL;
        }
    }
#endif



}