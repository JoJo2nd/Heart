/********************************************************************

    filename: 	memlogparser.cpp	
    
    Copyright (c) 14:11:2012 James Moran
    
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
#include "memlog.h"
#include "ioaccess.h"
#include <dbghelp.h>

typedef std::pair< std::string,std::string >    StrValuePair;
typedef std::vector< StrValuePair >             StrValuePairArray;

int resolveSymbols(uint64 address, uint64 oldbase, uint64 newbase);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool tokenize(InputStream* fis, char* outtoken, uint maxlen, uint* written)
{
    char ic;
    bool begun = false;
    
    *written = 0;

    do 
    {
        ic = fis->getChar();
        if (fis->getEOF()) return *written > 0 ? true : false;
        if (isspace(ic) && begun)
        {
            outtoken[*written] = '\0';
            return true;
        }
        else if (isspace(ic) && !begun) continue;
        else
        {
            begun = true;
            outtoken[(*written)++] = ic;
        }

    } while (*written < maxlen);

    outtoken[*written]='\0';
    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool tokenizePair(InputStream* fis, StrValuePair* sv)
{
    char ic;
    bool done = false;
    std::string* str = &sv->first;

    for (;;)
    {
        ic = fis->getChar();

        if (fis->getEOF()) return false;
        else if (ic == '\n' && !done) continue;
        else if (ic == '\r' && !done) continue;
        else if (ic == '\r' && done) return true;
        else if (ic == '\n' && done) return true;
        else if (ic == '(') str = &sv->second;
        else if (ic == ')') done = true;
        else str->push_back(ic);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int parseMemItem(InputStream* fis, MemLog* log, char* strbuffer, uint maxlen) 
{
    uint written;

    if (!tokenize(fis, strbuffer, maxlen, &written)) return -1; //Unexpected EOF
    
    if (strcmp(strbuffer, "MARKERPUSH") == 0)
    {
        if (!tokenize(fis, strbuffer, maxlen, &written)) return -1; //Unexpected EOF    
        log->pushMemoryMarker(strbuffer);
        return 0;
    }
    else if (strcmp(strbuffer, "MARKERPOP") == 0)
    {
        log->popMemoryMarker();
        return 0;
    }
    else if (strcmp(strbuffer, "ALLOC") == 0)
    {
        return 1;
    }
    else if (strcmp(strbuffer, "FREE") == 0)
    {
        return 2;
    }
    else if (strcmp(strbuffer, "SYMBOLTABLE") == 0)
    {
        return 3;
    }

    return -2; //Unexpected identifier %strbuffer
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int resolveAlloc(MemLog* log, const StrValuePairArray& strValueArray, uint64 oldbase, uint64 newbase)
{
    uint64 address = 0;
    uint64 size = 0;
    uint line = 0;
    std::string file;
    std::string heap; 
    Callstack backtrace;
    for (uint i = 0, iend = strValueArray.size(); i < iend; ++i)
    {
        if (strValueArray[i].first == "address")
        {
            if (sscanf_s(strValueArray[i].second.c_str(), "%llX", &address) != 1) return -3;//Failed to parse pair
        }
        else if (strValueArray[i].first == "heap")
        {
            heap = strValueArray[i].second;
        }
        else if (strValueArray[i].first == "size")
        {
            if (sscanf_s(strValueArray[i].second.c_str(), "%llX", &size) != 1) return -3;//Failed to parse pair
        }
        else if (strValueArray[i].first == "file")
        {
            file = strValueArray[i].second;
        }
        else if (strValueArray[i].first == "line")
        {
            if (sscanf_s(strValueArray[i].second.c_str(), "%u", &line) != 1) return -3;//Failed to parse pair
        }
        else if (strValueArray[i].first == "bt")
        {
            uint level;
            uint64 funcaddress;
            if (sscanf_s(strValueArray[i].second.c_str(), "%u,%llX", &level, &funcaddress) != 2) return -3;//Failed to parse pair
            backtrace.insertBacktraceLevel(level, funcaddress);
            resolveSymbols(funcaddress, oldbase, newbase);
        }
    }
    backtrace.sourcePath_ = file;
    backtrace.line_ = line;

    log->logMemoryAlloc(address, size, heap.c_str(), backtrace);

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int resolveFree(MemLog* log, const StrValuePairArray& strValueArray, uint64 oldbase, uint64 newbase)
{
    uint64 address;
    uint line = 0;
    std::string file("NotKnown");
    std::string heap; 
    Callstack backtrace;
    for (uint i = 0, iend = strValueArray.size(); i < iend; ++i)
    {
        if (strValueArray[i].first == "address")
        {
            if (sscanf_s(strValueArray[i].second.c_str(), "%llX", &address) != 1) return -3;//Failed to parse pair
        }
        else if (strValueArray[i].first == "heap")
        {
            heap = strValueArray[i].second;
        }
        else if (strValueArray[i].first == "file")
        {
            file = strValueArray[i].second;
        }
        else if (strValueArray[i].first == "line")
        {
            if (sscanf_s(strValueArray[i].second.c_str(), "%u", &line) != 1) return -3;//Failed to parse pair
        }
        else if (strValueArray[i].first == "bt")
        {
            uint level;
            uint64 funcaddress;
            if (sscanf_s(strValueArray[i].second.c_str(), "%u,%llX", &level, &funcaddress) != 2) return -3;//Failed to parse pair
            backtrace.insertBacktraceLevel(level, funcaddress);
            resolveSymbols(funcaddress, oldbase, newbase);
        }
    }
    backtrace.sourcePath_ = file;
    backtrace.line_ = line;

    log->logMemoryFree(address, heap.c_str(), backtrace);

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// int resolveSymbols(const StrValuePairArray& strValueArray)
// {
//     for (uint i = 0, iend = strValueArray.size(); i < iend; ++i)
//     {
//         if (strValueArray[i].first == "st")
//         {
//             size_t p = strValueArray[i].second.find_first_of(',');
//             if (p == std::string::npos) return -3; //Failed to parse
//             
//             uint64 address;
//             std::string sym = strValueArray[i].second.substr(p+1);
//             sscanf_s(strValueArray[i].second.c_str(), "%llX , ", &address);
// 
//             Callstack::insertSymbol(address, sym.c_str());
//         }
//     }
//     return 0;
// }

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int resolveSymbols(uint64 funcaddress, uint64 oldbase, uint64 newbase)
{
    uint64 address=funcaddress;
    address -= oldbase;
    address += newbase;
    if (Callstack::findSymbol(funcaddress)) {
        return 0;
    }

    char            buffer[2048+sizeof(SYMBOL_INFO)];
    char            outbuffer[4096];
    SYMBOL_INFO*    symbol=(SYMBOL_INFO*)buffer;
    DWORD           dwDisplacement;
    IMAGEHLP_LINE64 line;

    symbol->MaxNameLen   = 2047;
    symbol->SizeOfStruct = sizeof( SYMBOL_INFO );
    DWORD errorcode=0;
    HANDLE process=GetCurrentProcess();
    if (!SymFromAddr(process, (DWORD64)address, 0, symbol)) {
        errorcode=GetLastError();
    }
    if (SymGetLineFromAddr64(process, (DWORD64)address, &dwDisplacement, &line)) {
        if (errorcode!=0) {
            sprintf_s(outbuffer, sizeof(outbuffer), "0x%p(%u)", address, line.LineNumber);
        } else {
            sprintf_s(outbuffer, sizeof(outbuffer), "%s(%u)", symbol->Name, line.LineNumber);
        }
    } else {
        if (errorcode!=0) {
            sprintf_s(outbuffer, sizeof(outbuffer), "0x%p(NoLineNumber)", address);
        } else {
            sprintf_s(outbuffer, sizeof(outbuffer), "%s(NoLineNumber)", symbol->Name);
        }
    }
    Callstack::insertSymbol(funcaddress, outbuffer);

    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int parseMemLog(const char* filepath, MemLog* log, IODevice* ioaccess)
{
    #define MAX_STRBUF_SIZE (64*1024)
    InputStream* fis = ioaccess->openInputStream(filepath);
    char strbuffer[MAX_STRBUF_SIZE];
    uint written;
    int ret = 0;
    int itemtype = 0;
    StrValuePairArray valuePairs;

    //read the header
    uint64 baseaddress;
    char modulename[512];
    fis->read(&baseaddress, sizeof(baseaddress));
    fis->read(modulename, sizeof(modulename));
    modulename[sizeof(modulename)-1]=0;

    SymSetOptions(SYMOPT_DEFERRED_LOADS|SYMOPT_FAIL_CRITICAL_ERRORS|SYMOPT_ALLOW_ABSOLUTE_SYMBOLS|SYMOPT_UNDNAME|SYMOPT_DEBUG|SYMOPT_LOAD_LINES|SYMOPT_INCLUDE_32BIT_MODULES);
    if (SymInitialize(GetCurrentProcess(), NULL, true)==0) {
        return -1;
    }
    uint64 newbase=SymLoadModuleEx(GetCurrentProcess(), 0, modulename, 0, 0, 0, 0, 0);

    log->setBaseAddresses(baseaddress, newbase);
    log->pushMemoryMarker("PARSEBASE");

    while (tokenize(fis, strbuffer, MAX_STRBUF_SIZE, &written))
    {
        if (strcmp(strbuffer, "!!") == 0)
        {
            if (itemtype == 1)
            {
                resolveAlloc(log, valuePairs, baseaddress, newbase);
            }
            else if (itemtype == 2)
            {
                resolveFree(log, valuePairs, baseaddress, newbase);
            }
//             else if (itemtype == 3)
//             {
//                 resolveSymbols(valuePairs);
//             }

            valuePairs.clear();

            ret = parseMemItem(fis, log, strbuffer, MAX_STRBUF_SIZE);
            if (ret < 0) return ret;
            itemtype = ret;
        }
        else if (strlen(strbuffer) > 0)
        {
            fis->seek(-((int)strlen(strbuffer)+1), InputStream::Current);
            StrValuePair valuePair;
            ret = tokenizePair(fis, &valuePair);
            if (ret < 0) return ret;
            valuePairs.push_back(valuePair);
        }
    }

    if (itemtype == 1)
    {
        resolveAlloc(log, valuePairs, baseaddress, newbase);
    }
    else if (itemtype == 2)
    {
        resolveFree(log, valuePairs, baseaddress, newbase);
    }
//     else if (itemtype == 3)
//     {
//         resolveSymbols(valuePairs);
//     }

    log->popMemoryMarker();

    SymCleanup(GetCurrentProcess());

    return ret;
    #undef MAX_STRBUF_SIZE
}
