/********************************************************************

	filename: 	ioaccess.cpp	
	
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
