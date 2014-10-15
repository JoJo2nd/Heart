/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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