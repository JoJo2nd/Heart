/********************************************************************
	created:	2008/08/04
	created:	4:8:2008   14:19
	filename: 	sqbind_config.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef SQBIND_CONFIG_H__
#define SQBIND_CONFIG_H__

#include "new_on.h"

#define SQBIND_NEW( x )  NEW ( hVMHeap ) x
#define SQBIND_DELETE( x )  delete x
#define SQBIND_INLINE inline 
#ifdef HEART_DEBUG
#define SQBIND_DEBUG
#endif

#endif // _SQBIND_CONFIG_H__