/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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