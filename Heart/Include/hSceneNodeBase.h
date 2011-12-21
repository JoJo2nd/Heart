/********************************************************************
created:	2008/12/22

filename: 	SceneNodeBase.h

author:		James Moran

purpose:	
*********************************************************************/
#ifndef __HRSCENEGRAPHNODE_H__
#define __HRSCENEGRAPHNODE_H__

#include "hTypes.h"
#include "Common.h"
#include "hMath.h"
#include "hUtility.h"
#include "hRenderer.h"
#include "hResource.h"

class hiEffect;
class hiAnimation;

namespace Heart
{
	enum EngineSceneNodeTypeID
	{
		SCENENODETYPE_INVALID = -1,
		SCENENODETYPE_LOCATOR = 1,
		SCENENODETYPE_MESH,
		SCENENODETYPE_CAMERA,
		SCENENODETYPE_LIGHT,
	};

	enum LightType
	{
		LightType_DIRECTION,
		LightType_SPOT,
		LightType_POINT,

		LightType_MAX,
	};

	class hSceneGraphVisitorBase;

	class hSceneNodeBase : public hResourceClassBase
	{
	public:

		typedef hResourceHandle< hSceneNodeBase > SceneNodeRes;

		// to allow class use in linked list [12/24/2008 James]
		class ChildListNode : public hLinkedListElement< ChildListNode >
		{
		public:
			SceneNodeRes			child_;
		};

		struct TransformData
		{
			Heart::hMatrix		matrix_;
			Heart::hMatrix		globalMatrix_;
		};

		virtual void									Accept( hSceneGraphVisitorBase* guest ) = 0; // make all subclasses visitable enforced by the compiler

		static const hUint32							MAX_NAME_LEN = 256;
		static const hUint32							MAX_CHILDREN = 32;

		void											DefaultState();
		void											ResetAABB();
		void											InitTransform();
		void											Name( const hChar* name ) 
		{ 
			strcpy_s( &name_[ 0 ], MAX_NAME_LEN, name ); 
		}
		const hChar*									Name() const 
		{ 
			return &name_[ 0 ]; 
		}
		hUint32											GetTypeID() const 
		{
			return typeID_;
		}
		//TODO: re-implement some of these
// 		void											RotateX( hFloat rads )
// 		{
// 			Heart::Quaternion rot;
// 			Heart::Quaternion::rotateX( rads, rot );
// 			Heart::Quaternion::mult( rot, localRotation_, localRotation_ );
// 			MakeDirty();
// 		}
// 		void											RotateY( hFloat rads )
// 		{
// 			Heart::Quaternion rot;
// 			Heart::Quaternion::rotateY( rads, rot );
// 			Heart::Quaternion::mult( rot, localRotation_, localRotation_ );
// 			MakeDirty();
// 		}
// 		void											RotateZ( hFloat rads )
// 		{
// 			Heart::Quaternion rot;
// 			Heart::Quaternion::rotateZ( rads, rot );
// 			Heart::Quaternion::mult( rot, localRotation_, localRotation_ );
// 			MakeDirty();
// 		}
// 		void											SetRotation( const Heart::Quaternion& q )
// 		{
// 			localRotation_ = q;
// 			MakeDirty();
// 		}
// 		void											ApplyRotation( const Heart::Quaternion& q ) 
// 		{ 
// 			Heart::Quaternion::mult( q, localRotation_, localRotation_ ); 
// 			MakeDirty();
// 		}
// 		void											PostApplyRotation( const Heart::Quaternion& q ) 
// 		{ 
// 			Heart::Quaternion::mult( localRotation_, q, localRotation_ ); 
// 			MakeDirty();
// 		}
// 		void											RotateXYZ( const Heart::Vec3& r ) 
// 		{ 
// 			localRotationAngles_ = r; 
// 		}
// 		void											Move( const Heart::Vec3& vector )
// 		{
// 			localPosition_ += vector;
// 			MakeDirty();
// 		}
// 		void											Position( const Heart::Vec3& vector )
// 		{
// 			localPosition_ = vector;
// 			MakeDirty();
// 		}
// 		Heart::Vec3								Position() const
// 		{
// 			return localPosition_;
// 		}
// 		void											Scale( hFloat sx, hFloat sy, hFloat sz )
// 		{
// 			Heart::Vec3::set( sx, sy, sz, localScale_ );
// 			MakeDirty();
// 		}
// 		void											LocalMatrix( const Heart::Matrix& matrix ) 
// 		{ 
// 			localMatrix_ = matrix; 
// 			MakeDirty(); 
// 		};
// 		Heart::Matrix&							LocalMatrix()
// 		{
// 			if ( DirtyTransform_ == hTrue )
// 			{ 
// 				//Matrix tmpm, tmpm2, alrm;
// 				//Matrix iScale;
// 				Heart::Matrix Scale;
// 
// 				// build the local transform [1/20/2009 James]
// 				// 		if ( localScale_.x != 1.0f || localScale_.y != 1.0f || localScale_.z != 1.0f )
// 				// 		{
// 				// 			Matrix::scale( 1/localScale_.x, 1/localScale_.y, 1/localScale_.z, &iScale );
// 				// 			Matrix::scale( localScale_.x, localScale_.y, localScale_.z, &Scale );
// 				// 
// 				// 			Matrix::rotationFromQuaternion( localRotation_, &tmpm2 );
// 				// 			Matrix::mult( &iScale, &tmpm2, &tmpm );
// 				// 			Matrix::mult( &tmpm, &Scale, &iScale );
// 				// 			Matrix::mult( &iScale, &tmpm2, &localMatrix_ );
// 				// 		}
// 				// 		else
// 				// 		{
// 				// 			Matrix::rotationFromQuaternion( localRotation_, &localMatrix_ );
// 				// 		}
// 
// 				Heart::Matrix::scale( localScale_.x, localScale_.y, localScale_.z, &Scale );
// 				Heart::Matrix::rotationFromQuaternion( localRotation_, &localMatrix_ );
// 				Heart::Matrix::setTransform( localPosition_, &localMatrix_ );
// 				Heart::Matrix::mult( &Scale, &localMatrix_, &localMatrix_ );
// 
// 			}
// 
// 			return localMatrix_;
// 		}
// 		const Heart::Matrix&						GlobalMatrix() const
// 		{
// 			return globalMatrix_;
// 		}
// 		void											GlobalMatrix( Heart::Matrix& m ) 
// 		{ 
// 			globalMatrix_ = m; 
// 		}
// 		void											AttachChild( SceneNodeRes& pChild );
// 		void											RemoveChild( SceneNodeRes& pChild );
// 		void											DetachFromParent();
// 		hBool											IsAttachedToScene() const 
// 		{
// 			return parent_.HasData() || nChildren_ > 0;
// 		}
// 		void											MakeDirty() 
// 		{ 
// 			DirtyTransform_ = hTrue; 
// 		}
// 		void											MakeClean() 
// 		{ 
// 			DirtyTransform_ = hFalse; 
// 		}
// 		hBool											IsDirty() 
// 		{ 
// 			return DirtyTransform_; 
// 		}
// 		const Heart::AABB&						GetAABB() const
// 		{ 
// 			return globalAABB_; 
// 		}
// 		void											SetLocalAABB( const AABB& aabb )
// 		{
// 			AABB_ = aabb;
// 			MakeDirty();
// 		}
// 		const Heart::AABB&						GlobalAABB() const  
// 		{ 
// 			return finalGlobalAABB_; 
// 		}
// 		void											UpdateAABB() 
// 		{ 
// 			if ( DirtyTransform_ )
// 			{
// 				Heart::AABB::rotate( AABB_, globalMatrix_, globalAABB_ );
// 				finalGlobalAABB_ = globalAABB_;
// 			}
// 		}
// 		void											ExpandAABBBy( const Heart::AABB& a )
// 		{
// 			Heart::AABB::ExpandBy( finalGlobalAABB_, a );
// 		}
		const hMatrix*								GetMatrix() const { return &xf_[0].matrix_; }
		const hMatrix*								GetGlobalMatrix() const { return &xf_[0].globalMatrix_; }
		void										SetMatrix( const hMatrix* m ) { xf_[1].matrix_ = *m; }
		void										SetGlobalMatrix( const hMatrix* m ) { xf_[1].globalMatrix_ = *m; }
		const hAABB*								GetLocalAABB() const { return &localAABB_; }
		const hAABB*								GetGlobalAABB() const { return &globalAABB_; }
		SceneNodeRes&								pParent() 
		{ 
			return parent_; 
		}
		hUint32											nChildren() const 
		{ 
			return nChildren_; 
		}
		hLinkedList< ChildListNode >&					Children() 
		{ 
			return children_; 
		}
		void											MakeDirty() 
		{ 
			DirtyTransform_ = hTrue; 
		}
		void											MakeClean() 
		{ 
			DirtyTransform_ = hFalse; 
		}
		hBool											GetCastShadows() const { return castsShadow_; }
		void											SetCastShadows( hBool val ) { castsShadow_ = val; }
		void											AttachChild( SceneNodeRes& pChild );
		void											RemoveChild( SceneNodeRes& pChild );
		SceneNodeRes*									FindChild( const hChar* name );
		void											DetachFromParent();

#ifndef HEART_RESOURCE_BUILDER
	protected:
#endif // HEART_RESOURCE_BUILDER

		friend class									hSceneGraph;

		void											ApplyTransforms( const hMatrix* pm ) 
		{
			//GetMatrix calls always return idx 0 matrix
			//SetMatrix calls always return idx 1 matrix
			//< index 0 is current data, constant across a frame
			//< index 1 is to be applied data, mutable and applied to index 0 at beginning of a frame

			xf_[0].matrix_ = xf_[1].matrix_;
			hMatrix::mult( &xf_[1].matrix_, pm, &xf_[0].globalMatrix_ );
			//update AABB... etc
			Heart::hAABB::rotate( orginAABB_, xf_[0].globalMatrix_, localAABB_ );
			Heart::hAABB::rotate( orginAABB_, xf_[0].globalMatrix_, globalAABB_ );
		}

		void											UpdateAABB()
		{
			for ( ChildListNode* i = children_.GetHead(); i; i = i->GetNext() )
			{
				Heart::hAABB::ExpandBy( globalAABB_, i->child_->globalAABB_ );
			}
		}



		// only scene graph can create scene graph nodes [12/24/2008 James]
														hSceneNodeBase( hUint32 typeId );
		virtual											~hSceneNodeBase();

		virtual void									UnloadCallback() {}

		void											SetParent( const SceneNodeRes& parent );
		SceneNodeRes&									GetResourceHandle() { return us_; }
		void											SetResourceHandle( const SceneNodeRes& us ) { us_ = us; }
/*
		hUint32											typeID_;
		huArray< hChar, 32 >							name_;
		hUint32											nameCRC_;
		Heart::Matrix								localMatrix_;
		Heart::Matrix								globalMatrix_;
		Heart::Quaternion							appliedLocalRotation_;
		Heart::Quaternion							localRotation_;
		Heart::Vec3								localRotationAngles_;
		Heart::Vec3								localPosition_;
		Heart::Vec3								localScale_;

		SceneNodeRes									Us_;
		SceneNodeRes									parent_;
		hUint32											nChildren_;
		huLinkedList< ChildListNode >					pChildren_;
		Heart::AABB								globalAABB_;
		Heart::AABB								finalGlobalAABB_;///< AABB that includes the children of this node
		Heart::AABB								AABB_;//the local GetAABB of this object
*/

		// New Structure...
		hUint32											typeID_;
		hArray< hChar, MAX_NAME_LEN >					name_;
		hUint64											sceneID_;
		TransformData									xf_[2];//< index 0 is current data, constant across a frame
																//< index 1 is to be applied data, mutable and applied to index 0 at beginning of a frame

		//GetMatrix calls always return idx 0 matrix
		//SetMatrix calls always return idx 1 matrix
		//ApplyTransforms set idx 0 = idx 1 and updates global from parent i.e.
		//	ApplyTransforms( const Matrix pm ) 
		//	{
		//		xf_[0].matrix_ = xf[1].matrix_;
		//		xf_[0].globalMatrix_ = xf[1].matrix_ * pm;
		//		//update AABB... etc
		//	}

		Heart::hAABB								orginAABB_;//< local unrotated aabb
		Heart::hAABB								localAABB_;//< local aabb rotated from matrix_
		Heart::hAABB								globalAABB_;//< global aabb in world space, includes child aabb
		SceneNodeRes									us_;
		SceneNodeRes									parent_;
		hUint32											nChildren_;
		hLinkedList< ChildListNode >					children_;


		BEGIN_FLAGS()

		hBool DirtyTransform_ : 1;
		hBool DoDebugRender_ : 1;
		hBool castsShadow_ : 1;

		END_FLAGS()

	};
}

#endif //__HRSCENEGRAPHNODE_H__