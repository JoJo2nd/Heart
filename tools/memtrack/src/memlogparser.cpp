/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#include "memlog.h"
#include "ioaccess.h"
#include <windows.h>
#include <dbghelp.h>

int resolveSymbols(uint64 funcaddress, uint64 oldbase, uint64 newbase) {
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

int parseMemLog(const char* filepath, MemLog** log, IODevice* ioaccess) {
    InputStream* fis = ioaccess->openInputStream(filepath);
    if (!fis) {
        return 1;
    }
    *log = new MemLog(fis);
    return 0;
}
