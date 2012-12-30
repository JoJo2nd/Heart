/********************************************************************

	filename: 	ioaccess.h	
	
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
#pragma once

#ifndef IOACCESS_H__
#define IOACCESS_H__

class InputStream
{
public:
    enum SeekPos
    {
        Begin = SEEK_SET,
        Current = SEEK_CUR,
        End = SEEK_END,
    };
    virtual ~InputStream() {}
    virtual char getChar() = 0;
    virtual bool getEOF() = 0;
    virtual uint read(void* buffer, uint size) = 0;
    virtual int64 seek(int64 offset, SeekPos from) = 0;
    virtual uint64 tell() = 0;
    virtual uint64 size() = 0;
};

class IODevice
{
public:
    IODevice()
    {

    }
    ~IODevice()
    {

    }

    virtual InputStream* openInputStream(const char*);
    virtual void         closeInputStream(InputStream*);
};

#endif // IOACCESS_H__