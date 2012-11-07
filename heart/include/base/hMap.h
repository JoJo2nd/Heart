/********************************************************************

	filename: 	hMap.h	
	
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

#ifndef huMap_h__
#define huMap_h__

namespace Heart
{

    class hMemoryHeap;
	template< typename _Ky, typename _Ty > class hMap;

	template < typename _Ky, typename _Ty >
	class HEART_FORCEDLLEXPORT hMapElement
	{
	public:
		typedef _Ky			KeyType;
		typedef _Ty			Type;
		typedef _Ty*		TypePtr;
		typedef const _Ty*	ConstTypePtr;

		KeyType	GetKey() const { return key_; };
		TypePtr GetNext() 
		{
			TypePtr r = static_cast< TypePtr >(this);

			if ( !r->leftRight_[1] )
			{
				// Can't go right, go back until we are left of a parent
				while ( r->parent_ && r != r->parent_->leftRight_[0] ) { r = r->parent_; }
				r = r->parent_;
			}
			else
			{
				// branch node, go right, then left as far as possible
				r = r->leftRight_[1];
				hcAssert( r );
				while ( r->leftRight_[0] ) { r = r->leftRight_[0]; }
			}

			return r;
		}
		TypePtr GetPrev() 
		{
			TypePtr r = static_cast< TypePtr >(this);

			if ( !r->leftRight_[0] )
			{
				// Can't go left, go back until we are right of a parent
				while ( r->parent_ && r != r->parent_->leftRight_[1] ) { r = r->parent_; }
				r = r->parent_;
			}
			else
			{
				// branch node, go left one, and right as far as we can go
				r = r->leftRight_[0];
				hcAssert( r );
				while ( r->leftRight_[1] ) { r = r->leftRight_[1]; }
			}

			return r;
		}

	protected:

		friend class hMap< KeyType, Type >;

		hMapElement()
			: parent_( NULL )
			, colour_( TreeColour_MAX )
		{
			leftRight_[0] = NULL;
			leftRight_[1] = NULL;
		}
		~hMapElement()
		{
			hcAssertMsg( !leftRight_[0] && !leftRight_[1] && !parent_ && colour_ == TreeColour_MAX, "Deleting Map Node that is still in a Map Structure." );
		}

	private:

		enum TreeColour
		{
			TreeColour_RED,
			TreeColour_BLACK,

			TreeColour_MAX,
		};

		TreeColour	colour_;
		KeyType		key_;
		TypePtr		parent_;
		TypePtr		leftRight_[2];
	};

	////////////////////////////////////////////////////////////////////////////
	// Strongly based on tutorial work from : //////////////////////////////////
	// http://eternallyconfuzzled.com/tuts/datastructures/jsw_tut_rbtree.aspx //
	////////////////////////////////////////////////////////////////////////////
	template< typename _Ky, typename _Ty >
	class HEART_FORCEDLLEXPORT hMap
	{
	public:

		typedef _Ky 								KeyType;
		typedef _Ty 								Type;
		typedef _Ty* 								TypePtr;
		typedef hMapElement< KeyType, Type >		MapElement;
		typedef TypePtr								MapElementPtr;

		hMap() 
			: size_(0)
			, rbTreeRoot_(NULL)
			, deleteOnDestroy_(hTrue)
            , heap_(GetGlobalHeap())
		{
		}
        hMap(hMemoryHeap* heap)
            : size_(0)
            , rbTreeRoot_(NULL)
            , deleteOnDestroy_(hTrue)
            , heap_(heap)
        {
            
        }
		~hMap() 
		{
			Clear( deleteOnDestroy_ );
		}

        void            SetHeap(hMemoryHeapBase* heap) { heap_ = heap; }
		void			Insert( KeyType key, MapElementPtr val )
		{
			hcAssertMsg( !val->leftRight_[0] && !val->leftRight_[1] && !val->parent_, "Node belongs to another Map" );
			val->colour_ = MapElement::TreeColour_RED;
			val->key_ = key;
			val->leftRight_[0] = NULL;
			val->leftRight_[1] = NULL;
			
			rbTreeRoot_ = TreeInsert( val, rbTreeRoot_ );
			//Root is always black
			rbTreeRoot_->colour_ = MapElement::TreeColour_BLACK;

#ifdef HEART_MAP_CHECKS
            hcAssert(Validate());
#endif // HEART_MAP_CHECKS
			++size_;
		}
		MapElementPtr	Remove( KeyType key )
		{
			hBool done = hFalse;
			MapElementPtr rem = NULL;

			rbTreeRoot_ = TreeRemove( rbTreeRoot_, key, &done, &rem );
			if ( rbTreeRoot_ )
				rbTreeRoot_->colour_ = MapElement::TreeColour_BLACK;

			hcAssert(rem);

			BreakNodeLinks(rem);

#ifdef HEART_MAP_CHECKS
            hcAssert(Validate());
            hcAssert(!Find(key));
#endif // HEART_MAP_CHECKS

			--size_;

			return rem;
		}

		MapElementPtr	Remove( MapElementPtr i )
		{
			hBool done = hFalse;
			MapElementPtr rem = NULL;
            KeyType key = i->GetKey();

			//TODO: test that I can pass i instead of rbTreeRoot_
			rbTreeRoot_ = TreeRemove( rbTreeRoot_, i->GetKey(), &done, &rem );
			if ( rbTreeRoot_ )
				rbTreeRoot_->colour_ = MapElement::TreeColour_BLACK;

			hcAssert(rem && i == rem);
            
            BreakNodeLinks(rem);

#ifdef HEART_MAP_CHECKS
            hcAssert(Validate());
            hcAssert(!Find(key));
#endif // HEART_MAP_CHECKS

			--size_;

			return rem;
		}

		MapElementPtr	Erase( MapElementPtr i, MapElementPtr* n )
		{
			hBool done = hFalse;
			MapElementPtr rem = NULL;
			KeyType nk;
			hBool getnext = hFalse;
            KeyType key = i->GetKey();
			
			if ( i->GetNext() )
			{
				nk = i->GetNext()->GetKey();
				getnext = hTrue;
			}

			rbTreeRoot_ = TreeRemove( rbTreeRoot_, i->GetKey(), &done, &rem );
			if ( rbTreeRoot_ )
				rbTreeRoot_->colour_ = MapElement::TreeColour_BLACK;

			hcAssert(rem && i == rem);

			BreakNodeLinks(rem);

#ifdef HEART_MAP_CHECKS
            hcAssert(Validate());
            hcAssert(!Find(key));
#endif // HEART_MAP_CHECKS

			if ( getnext )
				*n = Find( nk );
            else
                *n = NULL;

			--size_;

			return rem;
		}

        TypePtr			Find( KeyType key )  const
        {
            MapElementPtr r = rbTreeRoot_;

            while ( r )
            {
                if ( key == r->GetKey() )
                    return r;

                r = r->leftRight_[r->GetKey() < key];
            }

            return NULL;
        }

		void			Clear( hBool deleteAll )
		{
			TreeClear(rbTreeRoot_, deleteAll);
			rbTreeRoot_ = NULL;
			size_ = 0; 
		}
		hUint32			GetSize() const { return size_; }
		TypePtr			GetHead() const 
		{ 
			MapElementPtr r = rbTreeRoot_;
			while( r && r->leftRight_[0] ) { r = r->leftRight_[0]; }
			return r; 
		}
		TypePtr			GetTail() const
		{
			MapElementPtr r = rbTreeRoot_;
			while( r && r->leftRight_[1] ) { r = r->leftRight_[1]; }
			return r; 
		}
		void			SetAutoDelete( hBool val ) { deleteOnDestroy_ = val; }
		hUint32			Validate() const { return ValidateR( rbTreeRoot_ ); }

	private:

		MapElementPtr GetGrandparent( MapElementPtr node ) 
		{
			if ( node && node->parent_ )
				return node->parent_->parent_;
			else 
				return NULL;
		}

		MapElementPtr GetUncle( MapElementPtr node )
		{
			MapElementPtr g = GetGrandparent( node );

			if ( !g )
				return NULL;
			else if ( node->parent_ == g->leftRight_[0] )
				return g->leftRight_[1];
			else
				return g->leftRight_[0];
		}

		void BreakNodeLinks( MapElementPtr rem ) 
		{
			rem->leftRight_[0] = NULL;
			rem->leftRight_[1] = NULL;
			rem->parent_ = NULL;
			rem->colour_ = MapElement::TreeColour_MAX;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////

		void TreeClear( MapElementPtr root, hBool deleteAll )
		{
			if ( !root )
				return;

			TreeClear(root->leftRight_[0], deleteAll);
			TreeClear(root->leftRight_[1], deleteAll);

			BreakNodeLinks( root );

            if (deleteAll)
            {
                hDELETE(heap_, root);
            }
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////

		hBool CheckRed( MapElementPtr n ) { return (n && n->colour_ == MapElement::TreeColour_RED); }

		MapElementPtr Rotate( MapElementPtr root, hUint32 direct )
		{
			MapElementPtr pivot = root->leftRight_[!direct];
			MapElementPtr t2 = pivot->leftRight_[direct];
			pivot->parent_ = root->parent_;
			pivot->leftRight_[direct] = root;
			root->parent_ = pivot;
			root->leftRight_[!direct] = t2;
			if ( t2 )
				t2->parent_ = root;

			// Swap colours
			root->colour_  = MapElement::TreeColour_RED;
			pivot->colour_ = MapElement::TreeColour_BLACK;

			return pivot;
		}
		
		MapElementPtr DoubleRotate( MapElementPtr root, hUint32 direct )
		{
			root->leftRight_[!direct] = Rotate( root->leftRight_[!direct], !direct );
			return Rotate( root, direct );
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////

		MapElementPtr TreeInsert( MapElementPtr node, MapElementPtr root )
		{
			if ( !root )
			{
				root = node;
				return root;
			}

			hUint32 direct = root->key_ < node->key_;
            hcAssertMsg( root->key_ != node->key_, "This map does not support duplicate entries" );
			node->parent_ = root;
			root->leftRight_[direct] = TreeInsert( node, root->leftRight_[direct] );

			if ( CheckRed( root->leftRight_[direct] ) )
			{
				if ( CheckRed( root->leftRight_[!direct] ) )
				{
					hcAssert( root->leftRight_[0] && root->leftRight_[!direct] );
					root->colour_					= MapElement::TreeColour_RED;
					root->leftRight_[0]->colour_	= MapElement::TreeColour_BLACK;
					root->leftRight_[1]->colour_	= MapElement::TreeColour_BLACK;
				}
				else
				{
					hcAssert( root->leftRight_[direct] );
					if ( CheckRed( root->leftRight_[direct]->leftRight_[direct] ) ) 
					{
						root = Rotate( root, !direct );
					}
					else if ( CheckRed( root->leftRight_[direct]->leftRight_[!direct] ) )
					{
						root = DoubleRotate( root, !direct );
					}
				}
			}
			return root;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////

		MapElementPtr TreeRemove( MapElementPtr root, KeyType rkey, hBool* comp, MapElementPtr* removed )
		{
			if ( !root )
			{
				*comp = hTrue;
				return root;
			}

			hUint32 direct = root->GetKey() < rkey;

			if ( root->GetKey() == rkey )
			{
				// If we only have one child, can just colour swap
				if ( !root->leftRight_[0] || !root->leftRight_[1] )
				{
					MapElementPtr str = root->leftRight_[!root->leftRight_[0]];

					if ( CheckRed( root ) )
					{
						*comp = hTrue;
					}
					else if ( CheckRed( str ) )
					{
						str->colour_ = MapElement::TreeColour_BLACK;
						*comp = hTrue; 
					}

                    *removed = root;

					if ( str )
						str->parent_ = root->parent_;
					return str;
				}
				else 
				{
					MapElementPtr next = root->GetPrev();		
                    direct = root->GetKey() < next->GetKey();

					// Copy next to root position in the graph, and put root into nexts position (a swap)
					// we then overwrite next's(that was root) key with new key, we then search for that new
					// new key id and delete that instead
					// This delete will result in the case above as next will only have
					// one child at most.
					SwapNodes(root, next);
                    root->key_ = next->GetKey();
	
					rkey = root->GetKey();
                    root = next;
				}
			}

			root->leftRight_[direct] = TreeRemove( root->leftRight_[direct], rkey, comp, removed );

			if ( !*comp )
			{
				root = TreeRemoveBalance( root, direct, comp );
			}

			return root;
		}

		MapElementPtr TreeRemoveBalance( MapElementPtr root, hUint32 direct, hBool* comp )
		{
			MapElementPtr parent = root;
			MapElementPtr sibling = root->leftRight_[!direct];

			if ( CheckRed( sibling ) )
			{
				root = Rotate( root, direct );
				sibling = parent->leftRight_[!direct];
			}

			if ( sibling )
			{
				if ( !CheckRed( sibling->leftRight_[0] ) && !CheckRed( sibling->leftRight_[1] ) )
				{
					if ( CheckRed( parent ) )
					{
						*comp = hTrue;
					}
					parent->colour_		= MapElement::TreeColour_BLACK;
					sibling->colour_	= MapElement::TreeColour_RED;
				}
				else
				{
					MapElement::TreeColour strc = parent->colour_;
					hBool newRoot = (root == parent);

					if ( CheckRed( sibling->leftRight_[!direct] ) )
					{
						parent = Rotate( parent, direct );
					}
					else
					{
						parent = DoubleRotate( parent, direct );
					}

					parent->colour_ = strc;
					parent->leftRight_[0]->colour_ = MapElement::TreeColour_BLACK;
					parent->leftRight_[1]->colour_ = MapElement::TreeColour_BLACK;

					if ( newRoot )
					{
						root = parent;
					}
					else
					{
						root->leftRight_[direct] = parent;
						if ( parent )
						{
							parent->parent_ = root;
						}
					}

					*comp = hTrue;
				}
			}

			return root;
		}

        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        
        void SwapNodes( MapElementPtr a, MapElementPtr b )
        {
            MapElementPtr p=a->parent_,l=a->leftRight_[0],r=a->leftRight_[1];
            MapElement::TreeColour tc = a->colour_;

            //special cases
            if (a->parent_ == b)
            {
                //swap node pointers
                a->parent_ = b->parent_;
                a->leftRight_[0] = b->leftRight_[0];
                a->leftRight_[1] = b->leftRight_[1];
                a->leftRight_[b->key_<a->key_] = b;
                a->colour_ = b->colour_;

                b->parent_ = a;
                b->leftRight_[0] = l;
                b->leftRight_[1] = r;
                b->colour_ = tc;
            }
            else if (b->parent_ == a)
            {
                //swap node pointers
                a->parent_ = b;
                a->leftRight_[0] = b->leftRight_[0];
                a->leftRight_[1] = b->leftRight_[1];
                a->colour_ = b->colour_;

                b->parent_ = p;
                b->leftRight_[0] = l;
                b->leftRight_[1] = r;
                b->leftRight_[a->key_<b->key_] = a;
                b->colour_ = tc;
            }
            else
            {
                //swap node pointers
                a->parent_ = b->parent_;
                a->leftRight_[0] = b->leftRight_[0];
                a->leftRight_[1] = b->leftRight_[1];
                a->colour_ = b->colour_;

                b->parent_ = p;
                b->leftRight_[0] = l;
                b->leftRight_[1] = r;
                b->colour_ = tc;
            }

            //update parent pointers
            if (a->parent_)
            {
                a->parent_->leftRight_[b->key_>a->parent_->key_] = a;
            }

            if (b->parent_)
            {
                b->parent_->leftRight_[a->key_>b->parent_->key_] = b;
            }

            if (a->leftRight_[0])
            {
                a->leftRight_[0]->parent_ = a;
            }
            if (a->leftRight_[1])
            {
                a->leftRight_[1]->parent_ = a;
            }

            if (b->leftRight_[0])
            {
                b->leftRight_[0]->parent_ = b;
            }
            if (b->leftRight_[1])
            {
                b->leftRight_[1]->parent_ = b;
            }

            hcAssert(a->leftRight_[0] != a && a->leftRight_[1] != a);
            hcAssert(b->leftRight_[0] != b && b->leftRight_[1] != b);
            hcAssert(a->parent_ != a);
            hcAssert(b->parent_ != b);
        }

		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////

		hUint32 ValidateR( MapElementPtr root ) const
		{
			if ( !root )
				return 1;

            if (root->leftRight_[0] && root->key_ < root->leftRight_[0]->key_)
            {
                hcBreak;//"Tree node order is malformed. Parent key is less than left child key");
            }
            if (root->leftRight_[1] && !(root->key_ < root->leftRight_[1]->key_))
            {
                hcBreak;//"Tree node order is malformed. Parent key is greater than right child key");
            }

			MapElementPtr ln = root->leftRight_[0];
			MapElementPtr rn = root->leftRight_[1];

			if ( root->colour_ == MapElement::TreeColour_RED )
			{
				if ( (ln && ln->colour_ == MapElement::TreeColour_RED) || (rn && rn->colour_ == MapElement::TreeColour_RED) )
				{
					hcBreak;//"Red-Black Tree: Red Violation!" );
					return 0;
				}
			}

			hUint32 lh = ValidateR( ln );
			hUint32 rh = ValidateR( rn );

			if ( lh > 0 && rh > 0 && lh != rh )
			{
				hcBreak;//"Red-Black Tree: Black Violation!" ); 
				return 0;
			}

			if ( lh > 0 && rh > 0 )
				return root->colour_ == MapElement::TreeColour_RED ? lh : lh + 1;
			else 
				return 0;
		}

		hUint32			    size_;
		MapElementPtr	    rbTreeRoot_;
		hBool			    deleteOnDestroy_;
        hMemoryHeapBase*    heap_;
	};

}
#endif // huMap_h__
