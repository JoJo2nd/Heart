/********************************************************************
	created:	2009/11/05
	created:	5:11:2009   20:25
	filename: 	SceneNodeLocator.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hSceneNodeLocator.h"
#include "Heart.h"
#include "hSceneGraphVisitorBase.h"
#include "hRenderer.h"


namespace Heart
{

	ENGINE_ACCEPT_VISITOR( hSceneNodeLocator, hSceneGraphVisitorEngine );

	hSceneNodeLocator::hSceneNodeLocator() :
		hSceneNodeBase( SCENENODETYPE_LOCATOR )
		,pRenderer( NULL )
	{
		ResetAABB();
	}

	hSceneNodeLocator::~hSceneNodeLocator()
	{

	}


	// void SceneNodeLocator::DebugRender( hrCamera& camera )
	// {
	// 	Heart::Vec3 pos;
	// 	pos.x = 0.0f;
	// 	pos.y = 0.0f;
	// 	pos.z = 0.0f;
	// 
	// 	//TODO: Fix locator debug rendering
	// 	#pragma message ("TODO: Fix locator debug rendering")
	// 	//pRenderer->NewRenderCommand< DebugDrawAABB >( &AABB_, pRenderer, hrColour() );
	// 	//pRenderer->NewRenderCommand< DebugDrawCross >( pRenderer, pos, &globalMatrix_, 5.0f, hrColour( 0.0f, 1.0f, 0.0f, 1.0f ) );
	// }
}