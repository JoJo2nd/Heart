/********************************************************************

	filename: 	SimpleRoomAddedLights.h	
	
	Copyright (c) 22:6:2011 James Moran
	
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

#ifndef SIMPLEROOMADDEDLIGHTS_H__
#define SIMPLEROOMADDEDLIGHTS_H__

#include "Heart.h"

namespace Heart
{
	class HeartEngine;
	class hSceneNodeBase;
	class hSceneNodeCamera;
	class hSceneNodeLight;
	class hSceneBundle;
}

class SimpleRoomAddedLights : public Heart::hStateBase
{
public:
	SimpleRoomAddedLights( const hChar* sceneResName, Heart::HeartEngine* engine ) 
		: hStateBase( "SimpleRoomAddedLight" )
		,sceneName_( sceneResName )
		,engine_( engine )
	{

	}
	~SimpleRoomAddedLights() {}

	virtual void				PreEnter();
	virtual hUint32				Enter();
	virtual void				PostEnter();
	virtual hUint32				Main();		
	virtual void				MainRender();
	virtual void				PreLeave();
	virtual hUint32				Leave();

private:

	typedef Heart::hResourceHandle< Heart::hSceneNodeBase > SceneNodeRes;
	//typedef Heart::hResourceHandle< Heart::SceneNodeLocator > LocatorNodeRes;
	typedef Heart::hResourceHandle< Heart::hSceneNodeLight > LightNodeRes;
	typedef Heart::hResourceHandle< Heart::hSceneNodeCamera > CameraNodeRes;
	typedef Heart::hResourceHandle< Heart::hSceneNodeMesh > MeshNodeRes;
	typedef Heart::hResourceHandle< Heart::hMaterial > MaterialRes;
	typedef Heart::hResourceHandle< Heart::hMesh > MeshRes;

	static const hUint32					NUM_LIGHTS = 5;

	const hChar*							sceneName_;
	Heart::HeartEngine*						engine_;
	LightNodeRes							lights_[NUM_LIGHTS];
	CameraNodeRes							cameraNode_;
	MeshNodeRes								sphereNode_;
	MeshRes									sphereMesh_;
	MaterialRes								solidColour_;
	Heart::hDeferredRenderVisitor		deferredVisitor_;

	Heart::hResourceHandle< Heart::hSceneBundle > sceneRes_;
};

#endif // SIMPLEROOMADDEDLIGHTS_H__