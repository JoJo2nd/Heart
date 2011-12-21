/********************************************************************
	created:	2009/10/30
	created:	30:10:2009   23:15
	filename: 	hGenHierarchy.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HCGENHIERARCHY_H__
#define HCGENHIERARCHY_H__

#include "hTypeList.h"

template <class _TList, template < class > class _Unit > 
class hGenScatterHierarchy; 

// GenScatterHierarchy specialization: Typelist to Unit 
template < class _T1, class _T2, template < class > class _Unit > 
class hGenScatterHierarchy< hTypeList< _T1, _T2 >, _Unit > : public hGenScatterHierarchy< _T1, _Unit >,
															   public hGenScatterHierarchy< _T2, _Unit > 
{ 
public: 
	typedef hTypeList< _T1, _T2 > TList; 
	typedef hGenScatterHierarchy< _T1, _Unit > LeftBase; 
	typedef hGenScatterHierarchy< _T2, _Unit > RightBase; 
}; 

// Pass an atomic type (non-typelist) to Unit 
template < class _AtomicType, template < class > class _Unit > 
class hGenScatterHierarchy : public _Unit< _AtomicType > 
{ 
	typedef _Unit< _AtomicType > LeftBase; 
}; 

// Do nothing for NullType 
template < template < class > class _Unit > 
class hGenScatterHierarchy< hNullType, _Unit > 
{ 
}; 

#endif // HCGENHIERARCHY_H__