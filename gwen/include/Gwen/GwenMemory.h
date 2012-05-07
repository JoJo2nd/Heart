/********************************************************************

	filename: 	GwenMemory.h	
	
	Copyright (c) 5:5:2012 James Moran
	
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
#ifndef GWENMEMORY_H__
#define GWENMEMORY_H__

#include <xmemory>

#ifdef HEART_ENGINE_LIB

extern void* GWENmalloc(size_t size);
extern void  GWENfree(void* ptr);

#define GWEN_OverrideNew() \
    public: \
    static void* operator new (size_t size) \
    { \
        return GWENmalloc(size); \
    } \
    static void operator delete (void *p) \
    { \
        GWENfree(p); \
    }

template< class _Ty >
class GwenSTLAllocator : public std::allocator< _Ty >
{	// generic allocator for objects of class _Ty
public:
    typedef typename _Ty			value_type;
    typedef value_type				*pointer;
    typedef value_type				& reference;
    typedef const value_type		*const_pointer;
    typedef const value_type		& const_reference;

    typedef size_t		size_type;
    typedef ptrdiff_t	difference_type;

    template< class _Other >
    struct rebind
    {	// convert an allocator<_Ty> to an allocator <_Other>
        typedef GwenSTLAllocator< _Other > other;
    };

    pointer address(reference _Val) const
    {	// return address of mutable _Val
        return (&_Val);
    }

    const_pointer address(const_reference _Val) const
    {	// return address of nonmutable _Val
        return (&_Val);
    }

    GwenSTLAllocator() _THROW0()
    {	// construct default allocator (do nothing)
    }

    GwenSTLAllocator(const GwenSTLAllocator<_Ty>&) _THROW0()
    {	// construct by copying (do nothing)
    }

    template<class _Other>
    GwenSTLAllocator(const GwenSTLAllocator<_Other>&) _THROW0()
    {	// construct from a related allocator (do nothing)
    }

    GwenSTLAllocator(const std::allocator<_Ty>&) _THROW0()
    {	// construct by copying (do nothing)
    }

    template<class _Other>
    GwenSTLAllocator(const std::allocator<_Other>&) _THROW0()
    {	// construct from a related allocator (do nothing)
    }


    template<class _Other>
    GwenSTLAllocator<_Ty>& operator=(const GwenSTLAllocator<_Other>&)
    {	// assign from a related allocator (do nothing)
        return (*this);
    }

    template<class _Other>
    GwenSTLAllocator<_Ty>& operator=(const std::allocator<_Other>&)
    {	// assign from a related allocator (do nothing)
        return (*this);
    }

    void deallocate(pointer _Ptr, size_type)
    {	// deallocate object at _Ptr, ignore size
        GWENfree(_Ptr);
    }

    pointer allocate(size_type _Count)
    {	// allocate array of _Count elements
        return static_cast<pointer>(GWENmalloc(sizeof(value_type)*_Count));
    }

    pointer allocate(size_type _Count, const void *)
    {	// allocate array of _Count elements, ignore hint
        return (allocate(_Count));
    }

    void construct(pointer _Ptr, const _Ty& _Val)
    {	// construct object at _Ptr with value _Val
        new (_Ptr) value_type(_Val);
    }

    void destroy(pointer _Ptr)
    {	// destroy object at _Ptr
        _Ptr->~value_type();
    }

    size_t max_size() const _THROW0()
    {	// estimate maximum array size
        size_t _Count = (size_t)(-1) / sizeof (_Ty);
        return (0 < _Count ? _Count : 1);
    }
};

template<class _Ty,
class _Other> inline
    bool operator==(const GwenSTLAllocator<_Ty>&, const GwenSTLAllocator<_Other>&) _THROW0()
{	// test for allocator equality (always true)
    return (true);
}

template<class _Ty,
class _Other> inline
    bool operator!=(const GwenSTLAllocator<_Ty>&, const GwenSTLAllocator<_Other>&) _THROW0()
{	// test for allocator inequality (always false)
    return (false);
}

#else

#define GWEN_OverrideNew()

template< class _Ty >
class GwenSTLAllocator : public std::allocator< _Ty >
{
public:
    template< class _Other >
    struct rebind
    {	// convert an allocator<_Ty> to an allocator <_Other>
        typedef GwenSTLAllocator< _Other > other;
    };

    GwenSTLAllocator() _THROW0()
    {	// construct default allocator (do nothing)
    }

    GwenSTLAllocator(const GwenSTLAllocator<_Ty>&) _THROW0()
    {	// construct by copying (do nothing)
    }

    template<class _Other>
    GwenSTLAllocator(const GwenSTLAllocator<_Other>&) _THROW0()
    {	// construct from a related allocator (do nothing)
    }

    GwenSTLAllocator(const std::allocator<_Ty>&) _THROW0()
    {	// construct by copying (do nothing)
    }

    template<class _Other>
    GwenSTLAllocator(const std::allocator<_Other>&) _THROW0()
    {	// construct from a related allocator (do nothing)
    }


    template<class _Other>
    GwenSTLAllocator<_Ty>& operator=(const GwenSTLAllocator<_Other>&)
    {	// assign from a related allocator (do nothing)
        return (*this);
    }

    template<class _Other>
    GwenSTLAllocator<_Ty>& operator=(const std::allocator<_Other>&)
    {	// assign from a related allocator (do nothing)
        return (*this);
    }
};

#endif

#endif // GWENMEMORY_H__