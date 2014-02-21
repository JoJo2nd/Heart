/********************************************************************

	filename: 	hStack.h	
	
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

#ifndef __HUSTACK_H__
#define __HUSTACK_H__

namespace Heart
{
	template< typename _Ty, hUint32 MaxSize >
	class hStack
	{
	public:

		hStack();
		virtual						~hStack();

		const _Ty*					top() const;
		_Ty*						top();
		void						push( const _Ty& val );
		void						pop();
		hUint32						size() const { return Top_; }
		void						clear() { Top_ = 0; }

	private:

		hUint32						Top_;
		hArray< _Ty, MaxSize >		StackArray_;

	};

	/*/////////////////////////////////////////////////////////////////////////

	function: pop - 

	purpose: 

	author: James Moran

	/////////////////////////////////////////////////////////////////////////*/
	template< typename _Ty, hUint32 MaxSize >
	void hStack<_Ty, MaxSize>::pop()
	{
		hcAssert( Top_ > 0 );

		--Top_;
	}

	/*/////////////////////////////////////////////////////////////////////////

	function: push - 

	purpose: 

	author: James Moran

	/////////////////////////////////////////////////////////////////////////*/
	template< typename _Ty, hUint32 MaxSize >
	void hStack<_Ty, MaxSize>::push( const _Ty& val )
	{
		hcAssert( Top_ < MaxSize );

		StackArray_[ Top_ ] = val;
		++Top_;

	}

	/*/////////////////////////////////////////////////////////////////////////

	function: top - 

	purpose: 

	author: James Moran

	/////////////////////////////////////////////////////////////////////////*/
	template< typename _Ty, hUint32 MaxSize >
	_Ty* hStack<_Ty, MaxSize>::top()
	{
		hcAssert( Top_ >= 1 );
		return &StackArray_[ Top_ - 1 ];
	}

	/*/////////////////////////////////////////////////////////////////////////

	function: top - 

	purpose: 

	author: James Moran

	/////////////////////////////////////////////////////////////////////////*/
	template< typename _Ty, hUint32 MaxSize >
	const _Ty* hStack<_Ty, MaxSize>::top() const
	{
		return &StackArray_[ Top_ - 1 ];
	}

	/*/////////////////////////////////////////////////////////////////////////

	function: ~huStack - 

	purpose: 

	author: James Moran

	/////////////////////////////////////////////////////////////////////////*/
	template< typename _Ty, hUint32 MaxSize >
	hStack<_Ty, MaxSize>::~hStack()
	{

	}

	/*/////////////////////////////////////////////////////////////////////////

	function: huStack - 

	purpose: 

	author: James Moran

	/////////////////////////////////////////////////////////////////////////*/
	template< typename _Ty, hUint32 MaxSize >
	hStack<_Ty, MaxSize>::hStack()
	{
		Top_ = 0;
	}
}

#endif //__HUSTACK_H__