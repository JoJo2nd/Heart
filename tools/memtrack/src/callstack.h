/********************************************************************

	filename: 	callstack.h	
	
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
#pragma once

#ifndef CALLSTACK_H__
#define CALLSTACK_H__

#include <map>

struct Callstack
{
    typedef std::vector<uint64>             BacktraceType;
    typedef std::map<uint64, std::string>   SymbolMapType;

    Callstack() 
        : line_(0)
    {
    }

    void            initilaise(const char* source, uint line);
    void            insertBacktraceLevel(uint level, uint64 address);

    static void     clearSymbolMap();
    static void     insertSymbol(uint64 address, const char* symbol);
    static bool     findSymbol(uint64 address);
    static bool     addressSymbolLookup(uint64 address, std::string* outsymbol);

    uint            line_;
    std::string     sourcePath_;
    BacktraceType   backtrace_;

    static SymbolMapType    s_symbolMap;
};

#endif // CALLSTACK_H__