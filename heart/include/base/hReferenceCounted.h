/********************************************************************

    filename: 	hIReferenceCounted.h	
    
    Copyright (c) 1:4:2012 James Moran
    
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


#ifndef HCIREFERENCECOUNTED_H__
#define HCIREFERENCECOUNTED_H__

namespace Heart
{
    class HEART_DLLEXPORT hIReferenceCounted 
    {
    public:
        hIReferenceCounted() 
        {
            reference_.value_ = 1;
        }
        virtual ~hIReferenceCounted() 
        {
            hcAssertMsg(reference_.value_ == 0, "Object instance deleted before all references were released (address=0x%p ref=%u)", this, reference_.value_);
        }
        void			AddRef()
        {
            hAtomic::Increment( reference_ );
        }
        void			DecRef()
        { 
            hcAssert(reference_.value_ > 0); 
            hAtomic::Decrement( reference_ ); 
            if ( reference_.value_ == 0 ) 
            { 
                OnZeroRef(); 
            } 
        }
        hUint32			GetRefCount() const { return reference_.value_; }
        
    protected:
        
        virtual void	OnZeroRef() {};
    private:
        hIReferenceCounted(hIReferenceCounted&) {}
        hIReferenceCounted& operator = (hIReferenceCounted&) {return *this;}

        hAtomicInt  reference_;
    };
}

#endif // HCIREFERENCECOUNTED_H__