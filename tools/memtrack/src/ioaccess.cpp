/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "precompiled.h"
#include "ioaccess.h"

class InputStreamImpl : public InputStream
{
public:
    InputStreamImpl(FILE* file)
        : file_(file)
    {
        assert(file_);
        fseek(file_, 0, SEEK_END);
        size_ = ftell(file_);
        rewind(file_);
    }

    ~InputStreamImpl() 
    {
    }

    char getChar()
    {
        return fgetc(file_);
    }

    bool getEOF()
    {
        return feof(file_) != 0;
    }

    uint read(void* buffer, uint size)
    {
        return fread(buffer, 1, size, file_);
    }

    int64 seek(int64 offset, SeekPos from) 
    {
        return fseek(file_, offset, (int)from);
    }

    uint64 tell()
    {
        return ftell(file_);
    }

    uint64 size()
    {
        return size_;
    }

private:
    friend class IODevice;

    uint64 size_;
    FILE* file_;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

InputStream* IODevice::openInputStream(const char* filepath)
{
    FILE* f;

    f = fopen(filepath, "rb");
    if (!f) return NULL;

    return new InputStreamImpl(f);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void IODevice::closeInputStream(InputStream* stream)
{
    InputStreamImpl* streamimpl = static_cast< InputStreamImpl* >(stream);
    if (!streamimpl) return;
    fclose(streamimpl->file_);

    delete streamimpl;
}
