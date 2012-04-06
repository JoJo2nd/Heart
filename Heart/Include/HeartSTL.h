/********************************************************************

	filename: 	HeartSTL.h	
	
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


#ifndef HEARTSTL_H__
#define HEARTSTL_H__

namespace Heart
{
    template< class _Ty, hMemoryHeap* pHeap >
    class HeapAllocator : public std::allocator< _Ty >
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
            typedef HeapAllocator< _Other, pHeap > other;
        };

        pointer address(reference _Val) const
        {	// return address of mutable _Val
            return (&_Val);
        }

        const_pointer address(const_reference _Val) const
        {	// return address of nonmutable _Val
            return (&_Val);
        }

        HeapAllocator() _THROW0()
        {	// construct default allocator (do nothing)
        }

        HeapAllocator(const HeapAllocator<_Ty,pHeap>&) _THROW0()
        {	// construct by copying (do nothing)
        }

        template<class _Other>
        HeapAllocator(const HeapAllocator<_Other,pHeap>&) _THROW0()
        {	// construct from a related allocator (do nothing)
        }

        template<class _Other>
        HeapAllocator<_Ty,pHeap>& operator=(const HeapAllocator<_Other,pHeap>&)
        {	// assign from a related allocator (do nothing)
            return (*this);
        }

        void deallocate(pointer _Ptr, size_type)
        {	// deallocate object at _Ptr, ignore size
            hHeapFree((*pHeap), _Ptr);
        }

        pointer allocate(size_type _Count)
        {	// allocate array of _Count elements
            return (pointer)hHeapMalloc((*pHeap),sizeof(value_type)*_Count);
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

	class string : public std::basic_string< char, std::char_traits<char>, HeapAllocator< char, &hGeneralHeap > >
	{
	public:
		string()
		{
		}
		string( const hChar* c ) :
			std::basic_string< char, std::char_traits<char>, HeapAllocator< char, &hGeneralHeap > >( c )
		{}
	private:
	};

	template< typename _Ty >
	class deque : public std::deque< _Ty, HeapAllocator< _Ty, &hGeneralHeap > >
	{
	public:
		deque() {}
	private:
		deque( const deque< _Ty >& ) {}
		deque& operator = ( const deque< _Ty >& ) {}
	};

}

#endif // HEARTSTL_H__