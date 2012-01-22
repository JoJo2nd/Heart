/********************************************************************
	created:	2010/06/27
	created:	27:6:2010   13:15
	filename: 	HeartSTL.h	
	author:		James
	
	purpose:	
*********************************************************************/

#ifndef HEARTSTL_H__
#define HEARTSTL_H__

#include "hMemory.h"
//JM TODO: remove
#include <vector>
#include <list>
#include <map>
#include <string>
#include <deque>
//JM: Replacements :D
#include "hArray.h"
#include "hLinkedList.h"
#include "hMap.h"

namespace Heart
{
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