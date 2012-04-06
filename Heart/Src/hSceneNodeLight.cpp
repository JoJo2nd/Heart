/********************************************************************

	filename: 	SceneNodeLight.cpp	
	
	Copyright (c) 27:3:2011 James Moran
	
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


namespace Heart
{

	ENGINE_ACCEPT_VISITOR( hSceneNodeLight, hSceneGraphVisitorEngine );

	hSceneNodeLight::hSceneNodeLight() :
		hSceneNodeBase( SCENENODETYPE_LIGHT )
	{
	}

	hSceneNodeLight::~hSceneNodeLight()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneNodeLight::SetMinRadius( hFloat minR )
	{
		minRadius_ = minR;
		if ( minRadius_ > maxRadius_ )
		{
			maxRadius_ = minRadius_;
		}

		BuildAABB();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneNodeLight::SetMaxRadius( hFloat maxR )
	{
		maxRadius_ = maxR;
		if ( minRadius_ > maxRadius_ )
		{
			maxRadius_ = minRadius_;
		}

		BuildAABB();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneNodeLight::SetSpotLightParameters( hFloat innerAngleRad, hFloat outerAngleRad, hFloat falloffdist )
	{
		outerAngle_ = hMin( outerAngleRad, hmPI / 3 );
		innerAngle_ = hMin( innerAngleRad, outerAngle_ );
		falloffDist_ = falloffdist;

		innerRadius_ = tan(innerAngle_)*falloffDist_;
		outerRadius_ = tan(outerAngle_)*falloffDist_;

		BuildAABB();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneNodeLight::BuildAABB()
	{
		if ( type_ == LightType_POINT )
		{
			orginAABB_.r_ = hVec3( maxRadius_, maxRadius_, maxRadius_ );
		}
		else if ( type_ == LightType_SPOT )
		{
            orginAABB_.c_ = hVec3( 0.0f, 0.0f, falloffDist_/2.0f );
            orginAABB_.r_ = hVec3( outerRadius_, outerRadius_, falloffDist_/2.0f );
		}
	}

}