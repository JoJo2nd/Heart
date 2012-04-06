/********************************************************************

	filename: 	hSceneNodeBase.cpp	
	
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


namespace Heart
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hSceneNodeBase::hSceneNodeBase( hUint32 typeId ) : 
		typeID_( typeId )
	{
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hSceneNodeBase::~hSceneNodeBase()
	{
#if 0
		hcAssertMsg( !parent_.HasData(), "SceneGraph node is attached to another scene node and is about to be deleted" );
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
#if 0

	void hSceneNodeBase::SetParent( const SceneNodeRes& parent )
	{
		if ( parent_.HasData() )
		{
			parent_.Release();
		}
		parent.Acquire();
		parent_ = parent;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneNodeBase::AttachChild( SceneNodeRes& pChild )
	{
		pChild->ApplyTransforms( GetGlobalMatrix() );
		//
		ChildListNode* pnewnode = hNEW ( hSceneGraphHeap ) ChildListNode();
		pChild.Acquire();
		pnewnode->child_ = pChild;

		pChild->SetParent( us_ );
		children_.PushBack( pnewnode );
		nChildren_ = children_.GetSize();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneNodeBase::RemoveChild( SceneNodeRes& pChild )
	{
		if ( pChild->parent_ == this )
		{
			ChildListNode* pnode = NULL;
			for ( ChildListNode* i = children_.GetHead(); i != NULL; i = i->GetNext() )
			{
				if ( i->child_ == pChild )
				{
					pnode = i;
				}
			}

			hcAssertMsg( pnode, "Scene node is a valid child but couldn't be found in the children list" );

			if ( pnode )
			{
				pChild->parent_.Release();
				children_.Remove( pnode );
				nChildren_ = children_.GetSize();

				pnode->child_.Release();
				delete pnode;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneNodeBase::DetachFromParent()
	{
		if ( parent_.HasData() )
		{
			parent_->RemoveChild( us_ );
		}
	}
#endif
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneNodeBase::DefaultState()
	{
// 		Matrix::identity( &xf_[0].matrix_ );
// 		Matrix::identity( &xf_[0].globalMatrix_ );
// 		Matrix::identity( &xf_[1].matrix_ );
// 		Matrix::identity( &xf_[1].globalMatrix_ );
		ResetAABB();


		nChildren_ = 0;
		MakeDirty();
	}

#if 0
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hSceneNodeBase::SceneNodeRes* hSceneNodeBase::FindChild( const hChar* name )
	{

		for ( hSceneNodeBase::ChildListNode* i = children_.GetHead(); i != NULL; i = i->GetNext() )
		{
			if ( strcmp( name, i->child_->Name() ) == 0 )
			{
				return &i->child_;
			}
			SceneNodeRes* ret = i->child_->FindChild( name );
			if ( ret )
			{
				return ret;
			}
		}

		return NULL;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// 21:12:05 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneNodeBase::ResetAABB()
	{
        orginAABB_.c_ = hVec3Func::zeroVector();
		globalAABB_.r_ = hVec3Func::zeroVector();
		localAABB_.c_ = hVec3Func::zeroVector();
		globalAABB_.r_ = hVec3Func::zeroVector();
		globalAABB_.c_ = hVec3Func::zeroVector();
        globalAABB_.r_ = hVec3Func::zeroVector();
	}

	//////////////////////////////////////////////////////////////////////////
	// 23:18:41 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneNodeBase::InitTransform()
	{
        for ( hUint32 i = 0; i < hStaticArraySize( xf_ ); ++i )
        {
            xf_[i].matrix_ = hMatrixFunc::identity();
		    xf_[i].globalMatrix_ = hMatrixFunc::identity();
        }

		castsShadow_ = hTrue;
		MakeDirty();
	}

}
