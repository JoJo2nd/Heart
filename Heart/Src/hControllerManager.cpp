/********************************************************************
	created:	2009/11/29
	created:	29:11:2009   1:03
	filename: 	hControllerManager.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hControllerManager.h"

namespace Heart
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hControllerManager::hControllerManager()
	{
		SetImpl( hNEW ( hGeneralHeap ) Device::InputDeviceManager() );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hControllerManager::~hControllerManager()
	{

	}

}
