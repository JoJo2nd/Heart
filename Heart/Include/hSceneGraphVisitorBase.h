/********************************************************************
	created:	2009/11/02
	created:	2:11:2009   21:15
	filename: 	SceneGraphVisitorBase.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HRSCENEGRAPHVISITORBASE_H__
#define HRSCENEGRAPHVISITORBASE_H__

#include "hTypes.h"

namespace Heart
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hSceneGraph;
	class hSceneNodeLocator;
	class hSceneNodeLight;
	class hSceneNodeMesh;
	class hSceneNodeCamera;
 
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hSceneGraphVisitorBase
	{
	public:

		hSceneGraphVisitorBase() 
			: abortChild_( hFalse )
		{

		}

		virtual void	PreVisit( hSceneGraph* /*pSceneGraph*/ ) {}
		virtual void	PostVisit( hSceneGraph* /*pSceneGraph*/ ) {}
		void			AbortChildVisit( hBool v ) { abortChild_ = v; }
		hBool			AbortChildVisit() const { return abortChild_; }
		virtual hBool 	IsEngineBase() { return hFalse; }

	private:

		hBool			abortChild_;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hSceneGraphVisitorEngine : public hSceneGraphVisitorBase
	{
	public:

		hSceneGraphVisitorEngine()
		{

		}

		virtual void	Visit( hSceneNodeLocator& visit ) {};
		virtual void	Visit( hSceneNodeMesh& visit ) {};
		virtual void	Visit( hSceneNodeCamera& visit ) {};
		virtual void	Visit( hSceneNodeLight& visit ) {};

	private:

	};

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define DEFINE_VISITABLE( nodeclass ) \
	virtual void Accept( Heart::hSceneGraphVisitorBase* guest ) { return AcceptImpl(*this, guest); } \
	void AcceptImpl( nodeclass& visited, Heart::hSceneGraphVisitorBase* guest );

#define ACCEPT_VISITOR( nodeclass, visitorbase ) \
	void nodeclass::AcceptImpl( nodeclass& visited, Heart::hSceneGraphVisitorBase* guest ) \
	{ \
		if ( guest->IsEngineBase() == hFalse )\
		{\
			visitorbase* pvis = static_cast< visitorbase* >( guest ); \
			pvis->Visit( visited ); \
		}\
	} 

#define ENGINE_ACCEPT_VISITOR( nodeclass, visitorbase ) \
	void nodeclass::AcceptImpl( nodeclass& visited, Heart::hSceneGraphVisitorBase* guest ) \
	{ \
		visitorbase* pvis = static_cast< visitorbase* >( guest ); \
		pvis->Visit( visited );\
	} 

#define ENGINE_DEFINE_VISITOR() \
	virtual hBool 	IsEngineBase() { return hTrue; }

#endif // HRSCENEGRAPHVISITORBASE_H__