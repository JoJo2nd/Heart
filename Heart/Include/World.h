/********************************************************************
	created:	2010/08/07
	created:	7:8:2010   15:37
	filename: 	World.h	
	author:		James
	
	purpose:	
*********************************************************************/

#include "hTypes.h"
#include "hVec2.h"

namespace Heart
{
namespace Physics2D
{

	class Body;

	//////////////////////////////////////////////////////////////////////////
	// A world class is a container for a collection of bodies which can /////
	// collide with each other. This class is heavily based on the world /////
	// class from box2D as it seemed a good starting point for me to begin ///
	// with. /////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	class World
	{
	public:
		World();
		~World();

		void		AddBody( Body* body );

		void		Step( hFloat delta );

		hVec2	    Gravity() const { return gravity_; }
		void		Gravity( const hVec2& val ) { gravity_ = val; }
		hUint32		Iterations() const { return iterations_; }
		void		Iterations( hUint32 val ) { iterations_ = val; }

	private:

		vector< Body* >		bodies_;

		hUint32				iterations_;
		hVec2			gravity_;
		
	};

}
}