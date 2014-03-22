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

namespace Heart
{
#pragma message ("STUBBED OUT: hEntity")
#if 0
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    struct hComponentContainer
    {
        hComponentContainer() 
            : componentHash_(0)
            , component_(NULL)
        {}
        hUint32     componentHash_;
        hComponent* component_;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    class hEntity : public hMapElement< hUint32, hEntity >
    {
    public:

        static const hUint32 ENTITY_NAME_MEX_LEN = 64;

        hEntity()
        {
            hZeroMem(name_, sizeof(name_));
        }
        ~hEntity()
        {

        }

        template< typename _Ty >
        _Ty* FindComponentByType()
        {
            _Ty* r = FindComponentByID(_Ty::GetComponentHash());
            return r;
        }
        hUint32          GetComponentCount() const { return components_.size(); }
        hComponent*      GetComponent(hUint32 idx) { return components_[idx].component_; }
        hComponent*      FindComponentByID(hUint32 id);
        void             AddComponent(hComponent* component);
        void             RemoveComponent(hComponent* component);
        hBool            GetCreated() const { return false; }
        void             SetCreated(hBool ) {}
        void             SetReserved(hBool ) {}
        hBool            GetReserved() const { return false; }
        const hChar*     GetName() { return name_; }
        void             SetName(const hChar* name) {hStrCopy(name_,ENTITY_NAME_MEX_LEN,name);}

    private:

        hChar                           name_[ENTITY_NAME_MEX_LEN];
        hVector< hComponentContainer >  components_;
        hBool                           created_    : 1;
        hBool                           persistant_ : 1;
    };
#endif
}

#endif // HENTITY_H__