/********************************************************************

	filename: 	SimpleRoomAddedLights.cpp	
	
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

#include "SimpleRoomAddedLights.h"
#include "Heart.h"

#define MOVE_SPEED ( 900.0f )

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void SimpleRoomAddedLights::PreEnter()
{
// 	engine_->GetResourceManager()->GetResource( sceneName_, sceneRes_ );
// 	engine_->GetResourceManager()->GetResource( "engine/materials/deferredsolid.mat", solidColour_ );
// 	deferredVisitor_.Initialise( engine_->GetRenderer(), engine_->GetResourceManager() );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 SimpleRoomAddedLights::Enter()
{
// 	hBool loaded = sceneRes_.IsLoaded();
// 	loaded &= solidColour_.IsLoaded();
// 	return loaded ? FINISHED : CONTINUE;
	return CONTINUE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void SimpleRoomAddedLights::PostEnter()
{
// 	using namespace Heart;
// 
// 	//Resources Ready, hook everything up
// 	engine_->GetSceneGraph()->CreateResourceNode( "sun", lights_[0], NEW ( hSceneGraphHeap ) Heart::hSceneNodeLight() );
// 	engine_->GetSceneGraph()->CreateResourceNode( "sun2", lights_[1], NEW ( hSceneGraphHeap ) Heart::hSceneNodeLight() );
// 	engine_->GetSceneGraph()->CreateResourceNode( "point", lights_[2], NEW( hSceneGraphHeap ) Heart::hSceneNodeLight() );
// 	engine_->GetSceneGraph()->CreateResourceNode( "spot1", lights_[3], NEW ( hSceneGraphHeap ) Heart::hSceneNodeLight() );
// 	engine_->GetSceneGraph()->CreateResourceNode( "spot2", lights_[4], NEW ( hSceneGraphHeap ) Heart::hSceneNodeLight() );
// 	engine_->GetSceneGraph()->CreateResourceNode( "debugCam", cameraNode_, NEW ( hSceneGraphHeap ) Heart::hSceneNodeCamera() );
// 
// 	lights_[0]->SetLightType( Heart::LightType_DIRECTION );
// 	lights_[1]->SetLightType( Heart::LightType_DIRECTION );
// 	lights_[2]->SetLightType( Heart::LightType_POINT );
// 	lights_[3]->SetLightType( Heart::LightType_SPOT );
// 	lights_[4]->SetLightType( Heart::LightType_SPOT );
// 
// 	lights_[0]->SetDiffuse( Heart::hColour( 1.0f, 1.0f, 1.0f, 1.0f ) );
// 	lights_[0]->SetLightPower( 0.25f );
// 	lights_[0]->SetSpecularExponent( 20 );
// 
// 	lights_[1]->SetDiffuse( Heart::hColour( 0.8f, 0.8f, 1.0f, 1.0f ) );
// 	lights_[1]->SetLightPower( 0.2f );
// 	lights_[1]->SetSpecularExponent( 30 );
// 
// 	lights_[2]->SetDiffuse( Heart::hColour( 0.8f, 0.45f, 0.64f, 1.0f ) );
// 	lights_[2]->SetLightPower( 0.3f );
// 	lights_[2]->SetSpecularExponent( 500 );
// 	lights_[2]->SetMinRadius( 200.0f );
// 	lights_[2]->SetMaxRadius( 400.0f );
// 
// 	lights_[3]->SetDiffuse( Heart::hColour( 1.0f, 1.0f, 1.0f, 1.0f ) );
// 	lights_[3]->SetLightPower( 1.0f );
// 	lights_[3]->SetSpecularExponent( 150 );
// 	lights_[3]->SetSpotLightParameters( hmDegToRad(10), hmDegToRad(19), 2000.0f );
// 
// 	lights_[4]->SetDiffuse( Heart::hColour( 0.8f, 0.8f, 1.0f, 1.0f ) );
// 	lights_[4]->SetLightPower( 1.0f );
// 	lights_[4]->SetSpecularExponent( 400 );
// 	lights_[4]->SetSpotLightParameters( hmDegToRad(10), hmDegToRad(35), 900.0f );
// 
// 	{
// 		hMatrix r,o;
// 		hMatrix::rotate( Heart::X_AXIS, hmDegToRad( 2 ), &r );
// 		hMatrix::mult( lights_[0]->GetMatrix(), &r, &o );
// 		lights_[0]->SetMatrix( &o );
// 	}
// 	{
// 		hMatrix r,o;
// 		hMatrix::rotate( Heart::X_AXIS, hmDegToRad( 30 ), &r );
// 		hMatrix::mult( lights_[1]->GetMatrix(), &r, &o );
// 		lights_[1]->SetMatrix( &o );
// 	}
// 	{
// 		hMatrix r,o;
// 		hMatrix::rotate( Heart::X_AXIS, hmDegToRad( 25 ), &r );
// 		hMatrix::mult( lights_[3]->GetMatrix(), &r, &o );
// 		o.m42 = 250.0f;
// 		lights_[3]->SetMatrix( &o );
// 	}
// 	{
// 		hMatrix r,o;
// 		hMatrix::rotate( Heart::X_AXIS, hmDegToRad( 10 ), &r );
// 		hMatrix::mult( lights_[4]->GetMatrix(), &r, &o );
// 		o.m42 = 260.0f;
// 		lights_[4]->SetMatrix( &o );
// 	}
// 
// 
// 	//Create a sphere
// 	//if ( !sphereMesh_.HasData() && solidColour_.IsLoaded() )
// 	{
// 		Heart::hResourceHandle< Heart::hIndexBuffer > ib;
// 		Heart::hResourceHandle< Heart::hVertexBuffer > vb;
// 		Heart::Utility::BuildSphereMesh( 32, 32, 70, engine_->GetRenderer(), &ib, &vb );
// 
// 		engine_->GetResourceManager()->CreateResource( "msh", sphereMesh_, NEW ( hRendererHeap ) Heart::hMesh(), "TestSphere" );
// 		sphereMesh_->SetIndexBuffer( ib );
// 		sphereMesh_->SetVertexBuffer( vb );
// 		sphereMesh_->SetMaterial( solidColour_ );
// 		//TODO: fix aabb generation from runtime resources
// 		Heart::hAABB ab;
// 		ab.c = Heart::ZeroVector3;
// 		ab.r[0] = 70.0f;
// 		ab.r[1] = 70.0f;
// 		ab.r[2] = 70.0f;
// 		sphereMesh_->SetAABB( ab );
// 
// 		// sphereMesh will acquire ref counts for these objects
// 		// we don't want them any more so let go..
// 		solidColour_.Release();
// 		ib.Release();
// 		vb.Release();
// 
// 		engine_->GetSceneGraph()->CreateResourceNode( "sphere", sphereNode_, NEW ( hSceneGraphHeap ) Heart::hSceneNodeMesh() );
// 		sphereNode_->AppendMesh( sphereMesh_ );
// 		//JM: This line shouldn't break but does...why?	
// 		//engine_->pSceneGraph()->pRootNode()->AttachChild( sphereNode_ );
// 		Heart::hMatrix m;
// 		Heart::hMatrix::transformation( Heart::hVec3( 30.0f, 150.0f, -30.0f ), &m );
// 		sphereNode_->SetMatrix( &m );
// 	}
// 
// 	SceneNodeRes& root = engine_->GetSceneGraph()->pRootNode();
// 	
// 	root->AttachChild( cameraNode_ );
// 	for ( hUint32 i = 0; i < NUM_LIGHTS; ++i )
// 	{
// 		root->AttachChild( lights_[i] );
// 	}
// 	root->AttachChild( *sceneRes_->GetRootNode() );
// 	root->AttachChild( sphereNode_ );
// 
// 	Heart::hViewport vp = 
// 	{ 0, 0, engine_->GetRenderer()->Width(), engine_->GetRenderer()->Height() };
// 
// 	cameraNode_->SetMatrix( &Heart::IdentityMatrix );
// 	cameraNode_->FovYDegrees( 45 );
// 	cameraNode_->Aspect( (hFloat)engine_->GetRenderer()->Width()/(hFloat)engine_->GetRenderer()->Height() );
// 	cameraNode_->Near( 50.0f );
// 	cameraNode_->Far( 7000.0f );
// 	cameraNode_->SetViewport( vp );
// 	cameraNode_->SetOrtho( hFalse );
// 
// 	hcPrintf( "Press \"TAB\" to close the console\nType Heart.NextTest() into console to view next test" );

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 SimpleRoomAddedLights::Main()
{
// 	using namespace Heart;
// 
// 	hFloat delta = Heart::hClock::Delta();
// 
// 	Heart::hSceneGraphUpdateVisitor updatevist;
// 	engine_->GetSceneGraph()->VisitScene( &updatevist, Heart::hSceneGraph::TOP_DOWN, true );
// 
// 	const Heart::hKeyboard* pKey = engine_->GetControllerManager()->GetSystemKeyboard();
// 	cameraNode_->UpdateCamera();
// 	hVec3 tLookDir = hVec3( cameraNode_->GetGlobalMatrix()->m31, cameraNode_->GetGlobalMatrix()->m32, cameraNode_->GetGlobalMatrix()->m33 );
// 	hVec3 tRight = hVec3( cameraNode_->GetGlobalMatrix()->m11, cameraNode_->GetGlobalMatrix()->m12, cameraNode_->GetGlobalMatrix()->m13 );
// 	hVec3 tUp = hVec3(0.0f, 1.0f, 0.0f );//cameraNode_->GetCamera().Up();
// 	hMatrix cammat = *cameraNode_->GetMatrix();
// 	hVec3 camp = hMatrix::GetTranslation( cammat );
// 	hMatrix::setTransform( ZeroVector3, &cammat );
// 	hVec3::normalise( tLookDir, tLookDir );
// 	hVec3::normalise( tRight, tRight );
// 
// 	if ( pKey->GetButtonPressed( Heart::Device::IID_KEYW ) )
// 	{
// 		camp += ( tLookDir * ( MOVE_SPEED * delta ) );
// 	}
// 	else if ( pKey->GetButtonPressed( Heart::Device::IID_KEYS ) )
// 	{
// 		camp += ( -tLookDir * ( MOVE_SPEED * delta ) );
// 	}
// 	else if ( pKey->GetButtonPressed( Heart::Device::IID_KEYA ) )
// 	{
// 		camp += ( -tRight * ( MOVE_SPEED * delta ) ) ;
// 	}
// 	else if ( pKey->GetButtonPressed( Heart::Device::IID_KEYD ) )
// 	{
// 		camp += ( tRight * ( MOVE_SPEED * delta ) );
// 	}
// 
// 	hFloat x = pKey->GetAxis( Heart::Device::IID_MOUSEXAXIS ).anologueVal_;
// 	hFloat y = pKey->GetAxis( Heart::Device::IID_MOUSEYAXIS ).anologueVal_;
// 
// 	{
// 		hMatrix r;
// 		hMatrix::rotate( tRight, -hmDegToRad( 2.5f )*delta*y, &r );
// 		hMatrix::mult( &cammat, &r, &cammat );
// 	}
// 	{
// 		hMatrix r;
// 		hMatrix::rotate( tUp, -hmDegToRad( 2.5f )*delta*x, &r );
// 		hMatrix::mult( &cammat, &r, &cammat );
// 	}
// 
// 	if ( pKey->GetButtonPressed( Heart::Device::IID_UPARROW ) )
// 	{
// 		hMatrix r;
// 		hMatrix::rotate( tRight, -hmDegToRad( 45 )*delta, &r );
// 		hMatrix::mult( &cammat, &r, &cammat );
// 	}
// 	else if ( pKey->GetButtonPressed( Heart::Device::IID_DOWNARROW ) )
// 	{
// 		hMatrix r;
// 		hMatrix::rotate( tRight, hmDegToRad( 45 )*delta, &r );
// 		hMatrix::mult( &cammat, &r, &cammat );
// 	}
// 	else if ( pKey->GetButtonPressed( Heart::Device::IID_LEFTARROW ) )
// 	{
// 		hMatrix r;
// 		hMatrix::rotate( tUp, -hmDegToRad( 45 )*delta, &r );
// 		hMatrix::mult( &cammat, &r, &cammat );
// 	}
// 	else if ( pKey->GetButtonPressed( Heart::Device::IID_RIGHTARROW ) )
// 	{
// 		hMatrix r;
// 		hMatrix::rotate( tUp, hmDegToRad( 45 )*delta, &r );
// 		hMatrix::mult( &cammat, &r, &cammat );
// 	}
// 
// 	hMatrix::setTransform( camp, &cammat );
// 	cameraNode_->SetMatrix( &cammat );
// 
// 	//Rotate some lights
// 	hMatrix lr,lo;
// 	hMatrix::rotate( Y_AXIS, hmDegToRad( 0.01f ), &lr );
// 	lights_[0]->SetMatrix( hMatrix::mult( lights_[0]->GetMatrix(), &lr, &lo ) );
// 
// 	hMatrix::rotate( Y_AXIS, hmDegToRad( 0.03f ), &lr );
// 	lights_[1]->SetMatrix( hMatrix::mult( lights_[1]->GetMatrix(), &lr, &lo ) );
// 
// 	hMatrix::rotate( Y_AXIS, hmDegToRad( 0.05f ), &lr );
// 	hMatrix::mult( lights_[3]->GetMatrix(), &lr, &lo );
// 	hMatrix::setTransform( hMatrix::GetTranslation( lights_[3]->GetMatrix() ), &lo );
// 	lights_[3]->SetMatrix( &lo );

	return CONTINUE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void SimpleRoomAddedLights::MainRender()
{
// 	deferredVisitor_.SetCamera( cameraNode_ );
// 	engine_->GetSceneGraph()->VisitScene( &deferredVisitor_, Heart::hSceneGraph::TOP_DOWN, false );
// 	deferredVisitor_.RenderGBuffers( engine_->GetRenderer(), engine_->GetSceneGraph() );
// 	deferredVisitor_.RenderLightPasses( engine_->GetRenderer(), engine_->GetSceneGraph() );
// 
// 	Heart::DebugRenderer::RenderDebugRenderCalls( engine_->GetRenderer(), cameraNode_ );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void SimpleRoomAddedLights::PreLeave()
{
// 	//Release Resouces held by this state...
// 	for ( hUint32 i = 0; i < NUM_LIGHTS; ++i )
// 	{
// 		if ( lights_[i].HasData() )
// 		{
// 			lights_[i]->DetachFromParent();
// 			lights_[i].Release();
// 		}
// 	}
// 
// 	if ( solidColour_.HasData() )
// 	{
// 		solidColour_.Release();
// 	}
// 
// 	if ( cameraNode_.HasData() )
// 	{
// 		cameraNode_->DetachFromParent();
// 		cameraNode_.Release();
// 	}
// 
// 	if ( sphereNode_.HasData() )
// 	{
// 		sphereNode_->DetachFromParent();
// 		sphereNode_.Release();
// 	}
// 
// 	if ( sphereMesh_.HasData() )
// 	{
// 		sphereMesh_.Release();
// 	}
// 
// 	if ( sceneRes_.HasData() )
// 	{
// 		sceneRes_.Release();
// 	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 SimpleRoomAddedLights::Leave()
{
	return FINISHED;
}
