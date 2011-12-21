/********************************************************************
	created:	2009/11/02
	created:	2:11:2009   21:51
	filename: 	SceneGraphUpdateVisitor.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HRSCENEGRAPHUPDATEVISITOR_H__
#define HRSCENEGRAPHUPDATEVISITOR_H__

#include "hSceneGraphVisitorBase.h"

namespace Heart
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hSceneGraphUpdateVisitor : public hSceneGraphVisitorEngine
	{
	public:
		
		hSceneGraphUpdateVisitor();
		virtual ~hSceneGraphUpdateVisitor();

		ENGINE_DEFINE_VISITOR();

		virtual void PreVisit( hSceneGraph* pSceneGraph );
		virtual void PostVisit( hSceneGraph*	pSceneGraph );
		virtual void Visit( hSceneNodeMesh& visit );
		virtual void Visit( hSceneNodeLocator& visit );
		virtual void Visit( hSceneNodeCamera& visit );
		
	private:

		hSceneGraph*			pSceneGraph_;
	};

}

#endif // HRSCENEGRAPHUPDATEVISITOR_H__