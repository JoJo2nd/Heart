/********************************************************************

	filename: 	hEntity.cpp	
	
	Copyright (c) 4:4:2012 James Moran
	
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

    hComponent* hEntity::FindComponentByID(hUint32 id)
    {
        for (hUint32 i = 0; i < components_.GetSize(); ++i)
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
        hcAssert(FindComponentByID(component->GetID()) == NULL);

        hComponentContainer c;
        c.component_ = component;
        c.componentHash_ = component->GetID();
        components_.PushBack(c);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hEntity::RemoveComponent( hComponent* component )
    {
        for (hUint32 i = 0; i < components_.GetSize(); ++i)
        {
            if(components_[i].component_ == component)
                components_[i].component_ = NULL;
        }
    }



}