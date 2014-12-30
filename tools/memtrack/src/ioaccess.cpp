/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#include "ioaccess.h"
#include <windows.h>
#include <assert.h>

class InputStreamImpl : public InputStream
{
public:
    InputStreamImpl(HANDLE in_file_handle)
        : fileHandle(in_file_handle)
        , mapping_size(0)
        , base(nullptr)
    {
        mapping_size = GetFileSize(in_file_handle, nullptr);
        DWORD protect = PAGE_READONLY;
        DWORD access = FILE_MAP_READ;
        DWORD sizehi = 0;
        DWORD sizelow = 0;
        DWORD offsethi = 0;
        DWORD offsetlow = 0;
        sizehi = (DWORD)(mapping_size >> 32);
        sizelow = (DWORD)mapping_size & 0xFFFFFFFF;

        mmaphandle = CreateFileMapping(in_file_handle, nullptr, protect, sizehi, sizelow, nullptr);
        assert(mmaphandle != INVALID_HANDLE_VALUE);
        base = MapViewOfFile(mmaphandle, access, offsethi, offsetlow, 0);
    }

    ~InputStreamImpl() {
        UnmapViewOfFile(base);
        CloseHandle(mmaphandle);
        CloseHandle(fileHandle);
    }

    virtual void* getBase() {
        return base;
    }

    uint64 size() {
        return mapping_size;
    }

private:
    friend class IODevice;

    HANDLE fileHandle;
    uint64 mapping_size;
    HANDLE mmaphandle;
    void*  base;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

InputStream* IODevice::openInputStream(const char* filepath)
{
    DWORD access = GENERIC_READ;
    DWORD share = 0;// < always ZERO, dont let things happen to file in use!
    LPSECURITY_ATTRIBUTES secatt = NULL;// could be a prob if passed across threads>?
    DWORD creation = OPEN_EXISTING;
    DWORD flags = FILE_ATTRIBUTE_NORMAL;
    HANDLE fhandle;

    fhandle = CreateFile(filepath, access, share, secatt, creation, flags, nullptr);

    if (fhandle == INVALID_HANDLE_VALUE) {
        return nullptr;
    }

    return new InputStreamImpl(fhandle);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void IODevice::closeInputStream(InputStream* stream)
{
    InputStreamImpl* streamimpl = static_cast< InputStreamImpl* >(stream);
    delete streamimpl;
}
