/********************************************************************
	created:	2010/08/07
	created:	7:8:2010   16:11
	filename: 	Body.h	
	author:		James
	
	purpose:	
*********************************************************************/

#include "hTypes.h"
#include "hVector.h"
#include "hPoly2.h"

namespace Heart
{
namespace Physics2D
{
	class World;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class Body
	{
	public:
		
		static const hFloat			STATIC_MASS;

		Body();
		virtual ~Body();

		void			AddImpluse( const hVec2& impluse ) { force_ += impluse; }

	protected:

		hVec2		position_;
		hVec2		velocity_;

		hFloat			rotation_;
		hFloat			angularVelocity_;
		
		hVec2		force_;
		hFloat			torque_;

		hFloat			friction_;
		hFloat			mass_;
		hFloat			invMass_;

		World*			pWorld_;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class Box : public Body
	{
	public:
		Box();
		~Box();

		void				Set( const hPoly2< 4 >& boxBody, hFloat mass );

	private:

		hPoly2< 4 >	boxBody_;
	};

}
}