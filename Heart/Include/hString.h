/********************************************************************

	filename: 	hString.h	
	
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
#ifndef HSTRING_H__
#define HSTRING_H__

namespace Heart
{
    /*
     * This will need to be replaced at some point, possibly after I write a free list or pool allocator
     */
    class hString : public std::basic_string< char, std::char_traits<char>, HeapAllocator< char, &hGeneralHeap > >
    {
    public:
        hString()
        {
        }
        hString( const hChar* c ) 
            : std::basic_string< char, std::char_traits<char>, HeapAllocator< char, &hGeneralHeap > >( c )
        {}
    private:
    };
}

#endif // HSTRING_H__