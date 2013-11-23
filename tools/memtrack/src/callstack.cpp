/********************************************************************

	filename: 	callstack.cpp	
	
	Copyright (c) 9:11:2012 James Moran
	
	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.
	
	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:
	
	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.
	
	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.
	
	3. This notice may not be removed or altered from any source
	distribution.

*********************************************************************/

#include "precompiled.h"
#include "callstack.h"


Callstack::SymbolMapType Callstack::s_symbolMap;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void Callstack::initilaise(const char* source, uint line)
{
    line = line_;
    sourcePath_ = source;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void Callstack::insertBacktraceLevel(uint level, uint64 address)
{
    if (level >= backtrace_.size())
    {
        backtrace_.resize(level+1);
    }
    backtrace_[level] = address;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void Callstack::clearSymbolMap()
{
    s_symbolMap.clear();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool Callstack::addressSymbolLookup(uint64 address, std::string* outsymbol)
{
    SymbolMapType::const_iterator ci = s_symbolMap.find(address);
    if (ci != s_symbolMap.end()) 
    {
        *outsymbol = ci->second;
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void Callstack::insertSymbol( uint64 address, const char* symbol )
{
    if (s_symbolMap.find(address) == s_symbolMap.end())
    {
        s_symbolMap[address] = symbol;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool Callstack::findSymbol(uint64 address) {
    SymbolMapType::const_iterator ci = s_symbolMap.find(address);
    if (ci != s_symbolMap.end()) 
    {
        return true;
    }
    return false;
}
