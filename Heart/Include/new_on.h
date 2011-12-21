/********************************************************************
	created:	2010/10/24
	created:	24:10:2010   11:35
	filename: 	new_on.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef NEW_ON_H__
#define NEW_ON_H__

#ifndef HEART_ALLOW_NEW
#ifdef HEART_DEBUG
#define hNEW( heap ) new ( heap, __FILE__, __LINE__ )
#else
#define hNEW( heap ) new ( heap )
#endif
#endif // HEART_ALLOW_NEW

#endif // NEW_ON_H__