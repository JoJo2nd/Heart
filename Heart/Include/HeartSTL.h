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
	template< typename _Ty >
	class vector : public std::vector< _Ty, HeapAllocator< _Ty, &hGeneralHeap > >
	{
	public:
		vector()
		{
		}
	private:
		//vector& operator = ( const vector< _Ty >& ) {}
	};

	template< typename _Ty >
	class list : public std::list< _Ty, HeapAllocator< _Ty, &hGeneralHeap > >
	{
	public:
		list()
		{
		}
	private:
		list( const list< _Ty >& ) {}
		list& operator = ( const list< _Ty >& ) {}
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


	template< typename _KeyTy, typename _ValTy >
	class map : public std::map< _KeyTy, _ValTy, std::less< _KeyTy >, HeapAllocator< std::pair< _KeyTy, _ValTy >, &hGeneralHeap > >
	{
	public:
		map()
		{
		}
	private:
		map( const map< _KeyTy, _ValTy >& ){}
		map& operator = ( const map< _KeyTy, _ValTy >& ) {}
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

	template< typename _Ty >
	class pimpl
	{
	public:

		pimpl() : pImpl_( NULL ) {}
		explicit pimpl( _Ty* pImpl ) : pImpl_( pImpl ) {}
		virtual ~pimpl() { delete pImpl_; pImpl_ = NULL; }
	
		void SetImpl( _Ty* pImpl ) { pImpl_ = pImpl; }
		_Ty* pImpl() { return pImpl_; }
		const _Ty* pImpl() const { return pImpl_; }

	private:

		_Ty* pImpl_;
	};
}

#endif // HEARTSTL_H__