/********************************************************************
	created:	2010/10/03
	created:	3:10:2010   11:57
	filename: 	SquirrelMemory.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

//////////////////////////////////////////////////////////////////////////
// Custom memory hooks for squirrels memory allocations //////////////////
//////////////////////////////////////////////////////////////////////////
#include "Common.h"
#define _INCLUDED_IN_SQUIRREL_WRAPPER_
#include "sqpcheader.h"
#undef _INCLUDED_IN_SQUIRREL_WRAPPER_
#include "hcMemory.h"

void *sq_vm_malloc(SQUnsignedInteger size)
{	
	//has a min alginment of 8
	return hVMHeap.alloc( size ); 
}

void *sq_vm_realloc(void *p, SQUnsignedInteger oldsize, SQUnsignedInteger size)
{ 
	return hVMHeap.reAlloc( p, size ); 
}

void sq_vm_free(void *p, SQUnsignedInteger size)
{	
	hVMHeap.release( p );
}
