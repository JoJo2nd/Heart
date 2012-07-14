/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/


#include "Gwen/Hook.h"

#ifdef GWEN_HOOKSYSTEM

using namespace Gwen;
using namespace Gwen::Hook;

HookList& Gwen::Hook::GetHookList()
{
    static list_t<BaseHook*>::type	g_HookList;
    return g_HookList;
}

void Gwen::Hook::AddHook( BaseHook* pHook )
{ 
	GetHookList().push_back( pHook ); 
}

void Gwen::Hook::RemoveHook( BaseHook* pHook )
{ 
	GetHookList().remove( pHook ); 
}

#endif