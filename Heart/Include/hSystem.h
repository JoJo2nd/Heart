/********************************************************************
	created:	2008/06/21
	created:	21:6:2008   12:29
	filename: 	System.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef hoWindow_h__
#define hoWindow_h__

#include "hTypes.h"
#include "hMap.h"
#include "DeviceKernel.h"

namespace Heart
{

class hSystem : public Device::Kernel
{
public:
	
	hSystem() 
	{
	}

	hBool										Create( const HeartConfig& config, EventManager* pEventManager );
	void										Update();
	void										Destroy();

private:

};

}


#endif // hoWindow_h__