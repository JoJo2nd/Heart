/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

namespace Heart
{

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

