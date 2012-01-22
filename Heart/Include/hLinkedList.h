/********************************************************************
	created:	2008/06/11
	created:	11:6:2008   21:32
	filename: 	huLinkList.h
	author:		James Moran
	
	purpose:	
*********************************************************************/

#ifndef huLinkList_h__
#define huLinkList_h__

#include "hTypes.h"

namespace Heart
{
	template< typename _Ty >
	class hLinkedList;

	template< typename _Ty >
	class hLinkedListElement
	{
	public:

		typedef _Ty		Type;
		typedef _Ty*	TypePtr;

		~hLinkedListElement() 
		{
			hcAssertMsg( !lnext_ && !lprev_, "Deleting element that still belongs to a list" );
		}
		TypePtr	GetNext() const { return lnext_; }
		TypePtr GetPrev() const { return lprev_; }

	protected:

		hLinkedListElement()
			: lnext_( NULL )
			, lprev_( NULL )
		{ }

	private:

		friend class hLinkedList< _Ty >;

		TypePtr		lnext_; 
		TypePtr		lprev_;
	};

	template< typename _Ty >
	class hLinkedList
	{
	public:

		typedef _Ty								Type;
		typedef hLinkedListElement< Type >		ElementType;
		typedef _Ty*							TypePtr;
		typedef TypePtr							ElementTypePtr;

		hLinkedList() 
			: size_( 0 )
			, head_( NULL )
			, tail_( NULL )
			, autoDelete_( hTrue )
		{
		}
		~hLinkedList()
		{
			Clear( autoDelete_ );
		}
		void					PushBack( ElementTypePtr add )
		{
			if ( !head_ )
			{
				// empty list [6/12/2008 James]
				AddHead( add );
			}
			else
			{
				tail_->lnext_ = (TypePtr)add;
				add->lprev_ = (TypePtr)tail_;
				add->lnext_ = NULL;
				tail_ = add;
			}
			++size_;
		}
		void					PushFront( ElementTypePtr add )
		{
			_Ty* newnode = add;

			if ( !head_ )
			{
				// is the first item in the list [6/11/2008 James]
				AddHead(newnode);
			}
			else
			{
				// push this item in front of the others [6/11/2008 James]
				head_->lprev_ = newnode;
				newnode->lnext_ = (TypePtr)head_;
				head_ = newnode;
				newnode->lprev_ = NULL;
			}
			++size_;
		}	
		void					InsertAfter( ElementTypePtr node, ElementTypePtr add )
		{
			add->lnext_ = (TypePtr)node->lnext_;
			if ( node->lnext_ )
			{
				node->lnext_->lprev_ = (TypePtr)add;
			}

			node->lnext_ = (TypePtr)add;
			add->lprev_ = (TypePtr)node;
		}
		void					InsertBefore( ElementTypePtr node, ElementTypePtr add )
		{
            if ( !node )
            {
                PushBack( add );
                return;
            }

			if ( node->lprev_ )
			{ 
				node->lprev_->lnext_ = (TypePtr)add;
			}

			add->lprev_ = (TypePtr)node->lprev_;

			node->lprev_ = (TypePtr)add;
			add->lnext_ = (TypePtr)node;

			if ( node == head_ )
			{
				head_ = (TypePtr)add;
			}

			++size_;

		}
		ElementTypePtr					Remove( ElementTypePtr node )
		{

			if ( node == head_ )
			{
				head_ = (TypePtr)head_->lnext_;
			}

			if ( node == tail_ )
			{
				tail_ = (TypePtr)tail_->lprev_;
			}

			if ( node->lprev_ )
			{
				node->lprev_->lnext_ = (TypePtr)node->lnext_;
			}
			if ( node->lnext_ )
			{
				node->lnext_->lprev_ = (TypePtr)node->lprev_;
			}

			--size_;
			BreakElementLinks(node);
			return (TypePtr)node;
		}
		void					Clear( hBool deleteAll ) 
		{ 
			while( head_ ) 
			{ 
				ElementTypePtr p = head_; 
				Remove( head_ ); 
				if ( deleteAll )
					delete p;
			}
		}
		hUint32					GetSize() const { return size_; }
		ElementTypePtr			GetHead() const { return (TypePtr)head_; }
		ElementTypePtr			GetTail() const { return (TypePtr)tail_; }
		void					SetAutoDelete( hBool val ) { autoDelete_ = hTrue; }

	private:

		void					AddHead( ElementTypePtr newnode )
		{
			head_ = newnode;
			tail_ = newnode;
			head_->lnext_ = NULL;
			tail_->lprev_ = NULL;
		}

		void BreakElementLinks( ElementTypePtr node ) 
		{
			node->lprev_ = NULL;
			node->lnext_ = NULL;
		}

		ElementTypePtr			head_;
		ElementTypePtr			tail_;
		hUint32					size_;
		hBool					autoDelete_;

	};
}

#endif // huLinkList_h__