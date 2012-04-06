/********************************************************************

	filename: 	SceneNodeLight.h	
	
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

#ifndef SCENENODELIGHT_H__
#define SCENENODELIGHT_H__


namespace Heart
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hSceneNodeLight : public hSceneNodeBase
	{
	public:

		hSceneNodeLight();
		virtual							~hSceneNodeLight();

		DEFINE_VISITABLE( hSceneNodeLight );

		void			SetLightType( LightType val ) { type_ = val; }
		LightType		GetLightType() const { return type_; }
		void			SetMinRadius( hFloat minR );
		hFloat			GetMinRadius() const { return minRadius_; }
		void			SetMaxRadius( hFloat maxR );
		hFloat			GetMaxRadius() const { return maxRadius_; }
		void			SetLightPower( hFloat power ) { power_ = power; }
		hFloat			GetLightPower() const { return power_; }
		hColour	GetDiffuse() const { return diffuse_; }
		void			SetDiffuse( const hColour& val ) { diffuse_ = val; }
		hColour	GetSpecular() const { return specular_; }
		void			SetSpecular( hColour& val ) { specular_ = val; }
		hFloat			GetSpecularExponent() const { return specExponent_; }
		void			SetSpecularExponent( hFloat val ) { specExponent_ = val; }
		void			SetSpotLightParameters( hFloat innerAngleRad, hFloat outerOuterRad, hFloat falloffdist );
		hFloat			GetInnerAngleRad() const { return innerAngle_; }
		hFloat			GetOuterAngleRad() const { return outerAngle_; }
		hFloat			GetSpotFalloffDist() const { return falloffDist_; }
		void			BuildAABB();

#ifndef HEART_RESOURCE_BUILDER
	private:
#endif // HEART_RESOURCE_BUILDER
		hColour	diffuse_;
		hColour	specular_;
 		LightType		type_;
		hFloat			power_;
		union
		{
			struct 
			{
				hFloat		minRadius_;//valid for point
				hFloat		maxRadius_;//valid for point
			};
			struct 
			{
				hFloat		innerAngle_;//valid for spot, in radians
				hFloat		outerAngle_;//valid for spot, in radians
				hFloat		falloffDist_;//valid for spot
				hFloat		innerRadius_;
				hFloat		outerRadius_;
			};
		};
		hFloat			specExponent_;

// 		hFloat			attMin_;//valid for point,spot
// 		hFloat			attMax_;//valid for point,spot
// 		hFloat			innerCone_;//valid for spot
// 		hFloat			outerCone_;//valid for spot
// 		Colour	diffuse_;
// 		Colour	specular_;
// 		hBool			castsShadows_;//valid for direction only, atm

	};

}

#endif // SCENENODELIGHT_H__