/********************************************************************

	filename: 	hEntity.h	
	
	Copyright (c) 22:1:2012 James Moran
	
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
#ifndef HENTITY_H__
#define HENTITY_H__

#include "hTypes.h"
#include "hComponent.h"

namespace Heart
{
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    struct hComponentContainer
    {
        hUint32     componentID_;
        hComponent* component_;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    class hEntity
    {
    public:

        static const hUint32 ENTITY_NAME_MEX_LEN = 64;

        hEntity()
        {

        }
        ~hEntity()
        {

        }

        template< typename _Ty >
        _Ty* FindComponentByType()
        {
                    
        }
        hUint32     GetComponentCount() const { return components_.GetSize(); }
        hComponent* GetComponent( hUint32 idx ) { return components_[idx].component_; }

    private:

        hChar                           name_[ENTITY_NAME_MEX_LEN];
        hVector< hComponentContainer >  components_;
    };
}

#endif // HENTITY_H__