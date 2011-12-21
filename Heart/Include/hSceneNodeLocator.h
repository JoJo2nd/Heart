/********************************************************************
	created:	2009/11/03
	created:	3:11:2009   20:42
	filename: 	SceneNodeLocator.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HRSCENEGRAPHNODELOCATOR_H__
#define HRSCENEGRAPHNODELOCATOR_H__


#include "hSceneNodeBase.h"
#include "hCamera.h"
#include "hSceneGraphVisitorBase.h"


namespace Heart
{
	class hRenderer;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hSceneNodeLocator : public hSceneNodeBase
	{
	public:

		hSceneNodeLocator();
		virtual							~hSceneNodeLocator();

		DEFINE_VISITABLE( hSceneNodeLocator );

		//void							DebugRender( hrCamera& camera );
		hRenderer*				pRenderer;

	private:

	};

}


#endif // HRSCENEGRAPHNODELOCATOR_H__