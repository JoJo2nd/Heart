/********************************************************************

	filename: 	SceneNode.h	
	
	Copyright (c) 16:1:2011 James Moran
	
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

#ifndef SCENENODE_H__
#define SCENENODE_H__

#include "stdafx.h"
#include <string>
#include <map>
#include <list>
#include "aiScene.h"
#include "wx\treectrl.h"
#include "AssImpWrapper.h"
#include "tinyxml.h"
#include "Serialiser.h"

class wxTreeItemId;

namespace HScene
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class VisitorBase;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	struct Property //: public Serialiser
	{
		enum PropType
		{
			TYPE_BOOL,
			TYPE_NUMBER,
			TYPE_STRING,

			TYPE_MAX,
		};

		void				Set( bool val );
		void				Set( float val );
		void				Set( const std::string& val );
		bool				Get( bool* pout ) const;
		bool				Get( float* pout ) const;
		bool				Get( std::string* pout ) const;
		PropType			Type() const { return type_; }

	private:
		std::string			name_;
		PropType			type_;
		bool				boolVal_;
		float				numberVal_;
		std::string			strVal_;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class Node : public Serialiser
	{
	public:

		typedef std::map< std::string, Property > PropertyMapType;
		typedef std::list< Node* > ChildListType;

		Node( const char* typeName ) :
			pParent_( NULL )
			,pTreeId_( NULL )
			,typeName_( typeName )
			,castShadow_( true )
		{
			D3DXMatrixIdentity( &matrix_ );
			D3DXMatrixIdentity( &globalMatrix_ );
			ClearAABB( &aabb_ );
		}
		virtual ~Node()
		{
			for ( ChildListType::iterator i = children_.begin(), e = children_.end(); i != e; ++i )
			{
				delete *i;
			}
			children_.clear();
			if ( pTreeId_ )
			{
				delete pTreeId_;
				pTreeId_ = NULL;
			}
		}

		virtual void			Accept( VisitorBase* guest ) = 0;

		const char*				NodeTypeName() const { return typeName_.c_str(); }
		void					OverrideTypeName( const char* typeName ) { typeName_ = typeName; }
		void					pName( const char* name ) { name_ = name; }
		const char*				pName() const { return name_.c_str(); }
		D3DXMATRIX*				pLocalMatrix() { return &matrix_; }
		void					pLocalMatrix( const D3DXMATRIX* m ) { matrix_ = *m; }
		const AABB*				GetAABB() { return &aabb_; } 
		Node*					pParent() const { return pParent_; }
		const ChildListType*	pChildren() const { return &children_; }
		void					AttachChild( Node* node )
		{
			ASSERT( node->pParent_ == NULL );
			node->pParent_ = this;
			children_.push_back( node );
		}
		void					RemoveChild( Node* node )
		{
			ASSERT( node->pParent_ == this );
			node->pParent_ = NULL;
			children_.remove( node );
		}
		Property*				GetProperty( const std::string& name );
		void					SetProperty( Property* prop );
		wxTreeItemId*			TreeItemID() const { return pTreeId_; }
		void					TreeItemID( wxTreeItemId* pitem ) 
		{ 
			if ( pTreeId_ )
			{
				delete pTreeId_;
				pTreeId_ = NULL;
			}
			pTreeId_ = pitem; 
		}
		
		D3DXMATRIX			globalMatrix_;
		D3DXMATRIX			matrix_;

		virtual bool		Serialise( TiXmlElement* toelement )
		{
			XML_LINK_STRING( name_, toelement );
			XML_LINK_STRING( typeName_, toelement );
			XML_LINK_AABB( aabb_, toelement );
			XML_LINK_MATRIX( matrix_, toelement );
			XML_LINK_MATRIX( globalMatrix_, toelement );
			XML_LINK_INT( castShadow_, toelement );

			return true;
		}
		virtual bool		Deserialise( TiXmlElement* fromelement )
		{
			XML_UNLINK_STRING( name_, fromelement );
			XML_UNLINK_STRING( typeName_, fromelement );
			XML_UNLINK_AABB( aabb_, fromelement );
			XML_UNLINK_MATRIX( matrix_, fromelement );
			XML_UNLINK_MATRIX( globalMatrix_, fromelement );
			XML_UNLINK_INT( castShadow_, fromelement );

			return true;
		}
		bool				GetCastShadows() const { return castShadow_ > 0; }
		void				SetCastShadows( bool castShadow ) { castShadow_ = castShadow; }

	protected:

		std::string			name_;
		std::string			typeName_;
		AABB				aabb_;
		u32					castShadow_;
		Node*				pParent_;
		wxTreeItemId*		pTreeId_;
		ChildListType		children_;
		PropertyMapType		propMap_;	//< for custom values
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	class NodeTreeHook : public wxTreeItemData
	{
	public:
		Node* pHook_;
	};
}

#endif // SCENENODE_H__