/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/


#ifndef HCIREFERENCECOUNTED_H__
#define HCIREFERENCECOUNTED_H__

namespace Heart
{
    class  hIReferenceCounted 
    {
    public:
        hIReferenceCounted() 
        {
            hAtomic::AtomicSet(reference_, 1);
        }
        virtual ~hIReferenceCounted() 
        {
            hcAssertMsg(hAtomic::AtomicGet(reference_) == 0, "Object instance deleted before all references were released (address=0x%p ref=%u)", this, hAtomic::AtomicGet(reference_));
        }
        hUint32 AddRef()
        {
            return hAtomic::Increment( reference_ );
        }
        hUint32 DecRef()
        { 
            hcAssert(hAtomic::AtomicGet(reference_) > 0); 
            auto rc = hAtomic::Decrement( reference_ );
            if ( rc == 0 ) { 
                OnZeroRef(); 
            } 
            return rc;
        }
        hUint32 GetRefCount() const { return hAtomic::AtomicGet(reference_); }
        
    protected:
        
        virtual void	OnZeroRef() {};
    private:
        hIReferenceCounted(hIReferenceCounted&) {}
        hIReferenceCounted& operator = (hIReferenceCounted&) {return *this;}

        hAtomicInt  reference_;
    };
}

#endif // HCIREFERENCECOUNTED_H__