/********************************************************************
	created:	2008/10/08
	created:	8:10:2008   20:07
	filename: 	TestBedCore.cpp
	author:		James
	
	purpose:	
*********************************************************************/

#include "Heart.h"

namespace Heart
{
	class HeartEngine;
	class hJob;
	class hSceneNodeLocator;
	class hSceneNodeLight;
	class hSceneBundle;
}

namespace Game
{
	class TerrainGenerator;
	class RenderShadowMapVisitor;

#define NUM_TEST_LIGHTS (10)

	class TestBedCore
	{
	public:
		TestBedCore();
		virtual ~TestBedCore();

		void							Initialise( const hChar* pCmdLine, Heart::HeartEngine* );
		void							EngineUpdateTick( hFloat tick, Heart::HeartEngine* pEngine );
		void							EngineRenderTick( hFloat tick, Heart::HeartEngine* pEngine );
		bool							EngineShutdownRequest( Heart::HeartEngine* pEngine );
		void							EngineShutdown( Heart::HeartEngine* pEngine );

		void							StartNextTest();

		static TestBedCore* 			pInstance() { return pInstance_; }

	private:

		typedef huFunctor< Heart::hStateBase* (*)() >::type UnitTestCreator;
		typedef Heart::hResourceHandle< Heart::hSceneNodeBase > SceneNodeRes;
		typedef Heart::hResourceHandle< Heart::hSceneNodeLocator > LocatorNodeRes;
		typedef Heart::hResourceHandle< Heart::hSceneNodeLight > LightNodeRes;
		typedef Heart::hResourceHandle< Heart::hSceneNodeCamera > CameraNodeRes;
		typedef Heart::hResourceHandle< Heart::hSceneNodeMesh > MeshNodeRes;
		typedef Heart::hResourceHandle< Heart::hMaterial > MaterialRes;
		typedef Heart::hResourceHandle< Heart::hMesh > MeshRes;

		static const hUint32					NUM_UNIT_TESTS = 1;

		Heart::hStateBase*						CreateSimpleLightRoom();
		Heart::hStateBase*						CreateTextureTestState();
		Heart::hStateBase*						CreateMapTestsState();
		Heart::hStateBase*						CreateListTestState();
        Heart::hStateBase*                      CreateReflectionTestState();
        Heart::hStateBase*						CreateResourceLoadTest();

		static int								LuaNextTest( lua_State* L );

		static TestBedCore*						pInstance_;
		Heart::HeartEngine*						pEngine_;

		//New Unit Test-Style Setup
		Heart::hStateBase*						currentTest_;
		hUint32									nextTest_;
		Heart::hVector< UnitTestCreator >		unitTestCreators_;
	};
}
