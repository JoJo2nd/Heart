/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/


#ifndef huLinkList_h__
#define huLinkList_h__

#include "base/hTypes.h"

namespace Heart
{
    template< typename _Ty >
    class hLinkedList;

    template< typename _Ty >
    class hLinkedListElement
    {
    public:

        typedef _Ty                      Type;
        typedef _Ty*                     TypePtr;
        typedef hLinkedListElement<_Ty>* NodePtr;

        ~hLinkedListElement() 
        {
            //hcAssertMsg( lnext_==this && lprev_==this, "Deleting element that still belongs to a list" );
        }
        TypePtr	GetNext() const { return static_cast<TypePtr>(lnext_); }
        TypePtr GetPrev() const { return static_cast<TypePtr>(lprev_); }

    protected:

        hLinkedListElement()
            : lnext_(this)
            , lprev_(this)
        { }

    private:

        friend class hLinkedList< _Ty >;

        NodePtr     lnext_; 
        NodePtr		lprev_;
    };

    template< typename _Ty >
    class hLinkedList
    {
    public:

        typedef _Ty								Type;
        typedef hLinkedListElement< Type >		NodeLink;
        typedef _Ty*							TypePtr;
        typedef TypePtr							NodePtr;
        typedef NodeLink                     Node;

        hLinkedList() 
            : size_( 0 )
        {
        }
        ~hLinkedList() {
            clear();
        }
        void pushBack( NodePtr add ) {
            addTail(add);
        }
        void pushFront(NodePtr add) {
            addHead(add);
        }	
        void insertAfter(NodePtr node, NodePtr add) {
            insert(node, add, add->lnext_);
        }
        void insertBefore( NodePtr node, NodePtr add ) {
            insert(node, add->lprev_, add);
        }
        NodePtr remove( NodePtr node ) {
            hcAssert(node && node->lprev_ && node->lnext_);
            NodeLink* next=node->lnext_;
            node->lprev_->lnext_ = node->lnext_;
            node->lnext_->lprev_ = node->lprev_;
            hcAssert(size_ > 0);
            --size_;
            breakElementLinks(node);
            return static_cast<NodePtr>(next);
        }
        void clear() { 
            if (size_ == 0) {
                return;
            }
            while(begin() != end()) { 
                remove(begin()); 
            }
        }
        hUint32 getSize() const { return size_; }
        const NodePtr begin() const { return root_.GetNext(); }
        const NodePtr end() const { return static_cast<NodePtr>(&root_); }
        NodePtr begin() { return root_.GetNext(); }
        NodePtr end() { return static_cast<NodePtr>(&root_); }
        void addHead(NodePtr newnode) {
            insert(newnode, static_cast<NodePtr>(&root_), root_.GetNext());
        }
        void addTail(NodePtr newnode) {
            insert(newnode, root_.GetPrev(), static_cast<NodePtr>(&root_));
        }
        void insert(NodePtr node, NodePtr before, NodePtr after) {
            //node must be removed from other lists first
            hcAssert(node->lprev_ == node && node->lnext_ == node);
            node->lprev_ = before;
            node->lnext_ = after;
            before->lnext_ = node;
            after->lprev_ = node;
            ++size_;
        }
        hBool existInList(NodePtr node) {
            for (NodePtr i=begin(), n=end(); i!=n; i=i->GetNext()) {
                if (node==i) {
                    return hTrue;
                }
            }
            return hFalse;
        }

    private:

        void breakElementLinks(NodePtr node) {
            node->lprev_ = node;
            node->lnext_ = node;
        }

        NodeLink root_;
        hUint size_;
    };

namespace hCircularLinkedList {
	template< typename t_ty >
	void breakElementLinks(t_ty* node) {
		node->lprev = node;
		node->lnext = node;
	}
	template< typename t_ty >
	void insert(t_ty* node, t_ty* before, t_ty* after) {
		//node must be removed from other lists first
		hcAssert(node->lprev == node && node->lnext == node);
		node->lprev = before;
		node->lnext = after;
		before->lnext = node;
		after->lprev = node;
	}
	template< typename t_ty >
	void insertAfter(t_ty* node, t_ty* add) {
		insert(node, add, add->lnext);
	}
	template< typename t_ty >
	void insertBefore(t_ty* node, t_ty* add) {
		insert(node, add->lprev, add);
	}
	template< typename t_ty >
	t_ty* remove(t_ty* node) {
		hcAssert(node && node->lprev && node->lnext);
		t_ty* next = node->lnext;
		node->lprev->lnext = node->lnext;
		node->lnext->lprev = node->lprev;
		breakElementLinks(node);
		return next;
	}
}
}

#endif // huLinkList_h__