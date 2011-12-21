/********************************************************************
	created:	2009/02/11

	filename: 	FBXImporter.h

	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef __FBXIMPORTER_H__
#define __FBXIMPORTER_H__

#include "FileImporter.h"
#include "Heart.h"
#include "PackerMesh.h"
#include "PackerSkeleton.h"
#include <stdio.h>
#include <vector>
#include <list>
#include <fbxsdk.h>

using namespace FBXFILESDK_NAMESPACE;

class FBXMaterial
{
public:

	void											buildMaterial( KFbxSurfaceMaterial* pmat, const std::string& filepath );
	hBool											getTextureFromProp( const char* type, KFbxSurfaceMaterial* pmat, const std::string& filepath  );
	KFbxObjectID									MatID_;	
	KFbxObjectID									UID_;
	std::list< std::string >						DiffuseTextures_;
	std::string										SpecularTexture_;
	std::string										BumpMap_;
	std::string										NoramlMap_;
	fbxDouble3										Ambient_;
	fbxDouble3										Diffuse_;
	fbxDouble3										Specular_;
	fbxDouble3										Emissive_;
	std::string										materialName_;
};

struct FBXKey
{
	hUint32											Time_;
	fbxDouble3										Value_;
};

struct FBXAnimNode
{
	typedef std::vector< FBXKey >					FBXKeyVector;
	typedef FBXKeyVector::const_iterator			FBXKeyVectorConstIter;

	FBXAnimNode()
	{
		TotalTimeMS_ = 0;
		nKeys_ = 0;
	}

	struct TimelineKey
	{
		hUint32										time_;
		FBXKey*										translationKey_;
		hUint32										translationKeyIdx_;
		FBXKey*										rotationKey_;
		hUint32										rotationKeyIdx_;
		FBXKey*										scaleKey_;
		hUint32										scaleKeyIndex_;
	};

	typedef std::vector< TimelineKey >				TimelineKeys;

	//util functions
	/* 
		returns the index of the respective key whose time is less than or equal
		to time passed
	*/
	hUint32											findTranslationKeyByTime( hUint32 time )
	{
		hUint32 size = Translation_.size();
		for ( hUint32 r = 0; r < size; ++r )
		{
			if ( Translation_[ r ].Time_ >= time )
			{
				 if ( r != 0 )
				 {
					 return r;
				 }
				 return r;
			}
		}

		return hErrorCode;
	}
	hUint32											findRotationKeyByTime( hUint32 time )
	{
		hUint32 size = Rotation_.size();
		for ( hUint32 r = 0; r < size; ++r )
		{
			if ( Translation_[ r ].Time_ >= time )
			{
				if ( r != 0 )
				{
					return r;
				}
				return r;
			}
		}

		return hErrorCode;
	}
	hUint32											findScaleKeyByTime( hUint32 time )
	{
		hUint32 size = Scale_.size();
		for ( hUint32 r = 0; r < size; ++r )
		{
			if ( Translation_[ r ].Time_ >= time )
			{
				if ( r != 0 )
				{
					return r;
				}
				return r;
			}
		}

		return hErrorCode;
	}

	hUint32											TotalTimeMS_;
	hUint32											nKeys_;				//< number of keys in the time line
	TimelineKeys									timeline_;			//< holds 
	FBXKeyVector									Translation_;
	FBXKeyVector									Rotation_;
	FBXKeyVector									Scale_;
};

class FBXHierNode
{
public:

	FBXHierNode()
	{
		pAnimation_ = NULL;
		pTransformLinks_ = NULL;
		pTransformLinksNames_ = NULL;
		TransformLinkRootBoneIdx_ = hErrorCode;

	}
	~FBXHierNode()
	{
		delete pAnimation_;
	}

	hUint32											type() const;
	std::string										meshName() const;
	hmMatrix										localMatrix() const;

//private:

	friend class FBXImporter;

	std::string										Name_;
	hUint32											localID_;
	hUint32											boneID_;
	hmVec3											Translation_;
	hmVec3											RotationAngles_;
	hmQuaternion									Rotation_;
	hmVec3											Scale_;
	hmMatrix										referenceMatrix_;
	hUint32											Type_;
	std::string										MeshName_;
	std::vector< std::string >						ChildNames_;
	std::vector< hUint32 >							ChildIdx_;
	FBXAnimNode*									pAnimation_;
	std::vector< hUint32 >*							pTransformLinks_;
	std::vector< std::string >*						pTransformLinksNames_;
	hUint32											TransformLinkRootBoneIdx_;
	KFbxNode*										pNode_;
};

class FBXImporter : public FileImporter
{
public:

	enum 
	{
		PARSE_HEIR,
		PARSE_MESH,
		PARSE_SKELETON,
		PARSE_ANIMATION,
		PARSE_MAX
	};

	enum 
	{
		TYPE_NODE,
		TYPE_MESH,
		TYPE_BONE,
		TYPE_LIGHT,
		TYPE_CAMERA,
		TYPE_TRANSFORM_LINK,
		TYPE_MAX,
	};
														FBXImporter();
														~FBXImporter();
	hBool												open( const hChar* name );
	hBool												readAllData();
	hBool												exportToPackerMesh( PackerMesh* pMesh );//= 0;
	//hBool												exportToPackerAnimation( PackerAnimation* pAnim ) = 0;
	FBXAnimNode*										getNodeAnimation( const char* pNodeName );
	void												close();
	std::string											getErrorString();
	hUint32												nTextures() const;
	const std::string&									getTexture( hUint32 i );
	hBool												hasBones() const;
	hBool												meshExists( const hChar* name );
	void												setAsActiveMesh( const hChar* name );
	hBool												isValidMatId( KFbxObjectID id );
	FBXMaterial											getMaterial( KFbxObjectID id );
	hUint32												nMaterials() const;
	const FBXMaterial*									pMaterial( hUint32 i ) const;
	const hUint32										getMaterialBuildID( KFbxObjectID id ) const;
	void												namePrefix( std::string& str );
	std::vector< FBXHierNode >&							hierarchyArray();
	void												fillAnimatedNodeArray( std::vector< FBXHierNode* >& list );
	FBXHierNode*										pHierarchyNodeByName( std::string& name )
	{
		std::vector< FBXHierNode >::iterator lend = HeirArchArray_.end();
		for ( std::vector< FBXHierNode >::iterator i = HeirArchArray_.begin(); i != lend; ++i )
		{
			if ( i->Name_ == name )
			{
				return &(*i);
			}
		}

		return NULL;
	}

private:

	class FBXMesh;
	class FBXSkeleton;
	class FBXAnimation;

	void												buildFromScene( KFbxNode* pNode, hUint32 parseOrder  );

	void extractAnimationFromNode( KFbxNode* pNode );
	void												AddTimelineForTime( FBXAnimNode::TimelineKey &tlKey, FBXAnimNode* pAnimNode );
	void												GetRotationKeysAtTime( KTime &time, KFbxNode* pNode, KFbxNode* pParent, FBXAnimNode* pAnimNode, hBool pushtoback = hTrue );
	hBool												extractMesh( KFbxNode* pNode );// extract 
	hBool												extractSkeleton( KFbxNode* pNode );
	void												postHeirarchyFixup();
	KFbxXMatrix											GetGeometry(KFbxNode* pNode) ;
	hmMatrix											ConvertRHMatrixToLHMatrix( const KFbxMatrix& rhMat );

	class FBXMesh
	{
	public:

		hBool											buildMesh( FBXImporter* pOwner, KFbxSdkManager* pSdkManager, KFbxScene* pScene, KFbxNode* pNode );
		hBool											getMaterialsForMesh();
		hBool											getTextureFromProp( KFbxProperty &prop, int i );

		FBXImporter*									pOwner_;
		std::string										MeshName_;
		KFbxSdkManager*									pSdkManager_;
		KFbxScene*										pScene_;
		KFbxMesh*										pMeshNode_;
		KFbxMesh*										pMeshTriNode_;
		std::vector< std::string >						Textures_;
		KFbxObjectID									MatID_;
		FBXSkeleton*									pSkeleton_;
		hUint32											SkeletonIndex_;
		hBool											HasBones_;
		hUint32											MaterialID_;
		AABB											AABB_;
		
	};

	//TODO: remove
	class FBXSkeleton 
	{
	public:
														FBXSkeleton();
		hBool											buildSkeleton( FBXImporter* pOwner, KFbxSdkManager* pSdkManager, KFbxScene* pScene, KFbxNode* pNode, hUint32* retidx = NULL );
		void											addBone( KFbxSkeleton* pParent, KFbxSkeleton* pSkel );
		void											sortBones();
		hBool											findBone( const hChar* name );
		hUint32											getBoneID( const hChar* name );

	private:

		friend class									FBXImporter;

		hUint32											newID();

		struct FBXBone
		{
			FBXBone() 
			{
				pCluster_ = NULL;
				pParent_ = NULL;
			}
			KFbxNode*									pNodeData_;
			KFbxNode*									pParent_;
			KFbxCluster*								pCluster_;
			std::string									Name_;
			hUint32										BoneID_;
			hUint32										nChildren_;
			std::vector< hUint32 >						ChildIndex_;
			KFbxVector4									T_;
			KFbxVector4									S_;
			KFbxVector4									R_;
			KFbxXMatrix									Global_;
			KFbxXMatrix									Local_;
			KFbxXMatrix									Parent_;
			FBXAnimNode									Animation_;
		};

		std::vector< FBXBone >							Bones_;
		std::map< std::string, hUint32 >				BoneID_;// to speed up the search [2/12/2009 James]
		FBXImporter*									pOwner_;
		KFbxSdkManager*									pSdkManager_;
		KFbxScene*										pScene_;
		hUint32											NextID_;

	};

private:

	void												getPreRotationMatrix( FBXSkeleton* skel, FBXSkeleton::FBXBone* bone, const char* name, hmMatrix& out, hmMatrix* parent = NULL );

	KFbxSdkManager*										pSdkManager_;
	KFbxScene*											pScene_;
	KFbxImporter*										pImporter_;
	KFbxStreamOptionsFbxReader*							pImportOptions_;
	int													FileFormat_;

	std::string											ErrorString_;

	FBXMesh*											pActiveMesh_;
	std::string											DefaultTextureName_;
	std::string											FilePath_;
	std::string											NamePrefix_;
	KString												AnimationTakeName_;
	hUint32												globalBoneId_;
	std::vector< std::string >*							pTransformLink_;

	std::vector< FBXMesh >								MeshArray_;
	std::vector< FBXSkeleton >							SkeletonArray_;
	std::vector< FBXMaterial >							MaterialArray_;
	std::vector< FBXHierNode >							HeirArchArray_;
};

inline FBXImporter::FBXImporter() : 
	pActiveMesh_( NULL ),
	DefaultTextureName_( "BogusTexture.png" ),
	NamePrefix_( "" )
{

}

inline std::string FBXImporter::getErrorString()
{
	return ErrorString_;
}

inline hUint32 FBXImporter::nTextures() const
{
	if ( pActiveMesh_ )
	{
		return static_cast< hUint32 >( pActiveMesh_->Textures_.size() );
	}
	return 0;
}

inline const std::string& FBXImporter::getTexture( hUint32 i )
{
	if ( pActiveMesh_ )
	{
		return pActiveMesh_->Textures_[ i ];
	}
	return DefaultTextureName_;
}

inline hBool FBXImporter::hasBones() const
{
	if ( pActiveMesh_ )
	{
		return pActiveMesh_->HasBones_;
	}
	return hFalse;
}

inline FBXImporter::FBXSkeleton::FBXSkeleton() : 
	NextID_( 0 )
{

}

inline hUint32 FBXImporter::FBXSkeleton::newID()
{
	return NextID_++;
}

inline void FBXImporter::setAsActiveMesh( const hChar* name )
{
	pActiveMesh_ = NULL;

	for ( hUint32 i = 0; i < MeshArray_.size(); ++i )
	{
		if ( MeshArray_[ i ].MeshName_ == name )
		{
			pActiveMesh_ = &MeshArray_[ i ];
			//hcAssert( pActiveMesh_->pMeshTriNode_->CheckIfVertexNormalsCCW() );
			break;
		}
	}
}

inline void FBXImporter::namePrefix( std::string& str )
{
	NamePrefix_ = str;
}

inline std::vector< FBXHierNode >& FBXImporter::hierarchyArray()
{
	return HeirArchArray_;
}

#endif //__FBXIMPORTER_H__