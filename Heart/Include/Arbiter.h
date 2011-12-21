/********************************************************************
	created:	2010/08/07
	created:	7:8:2010   16:07
	filename: 	Arbiter.h	
	author:		James
	
	purpose:	
*********************************************************************/

#include "hTypes.h"
#include "hVector.h"
#include "hmOBB2.h"

namespace Heart
{
namespace Physics2D
{
	class Body;

	class Contact
	{

	};

	class Arbiter
	{
	public:

		static const MAX_CONTACTS = 2;

		Arbiter( Body* b1, Body* b2 );
		~Arbiter();

		void	Update();

		Contact			contacts_[ MAX_CONTACTS ];
		hUint32			nContacts_;

		Body*			body1_;
		Body*			body2_;

		// Combined friction
		hFloat			friction_;
	};
}
}