/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
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
