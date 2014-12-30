/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "memtracktypes.h"

class InputStream
{
public:
    virtual ~InputStream() {}
    virtual uint64 size() = 0;
    virtual void*  getBase() = 0;
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
