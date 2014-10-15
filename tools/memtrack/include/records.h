/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#ifndef RECORDS_H__
#define RECORDS_H__

#include "callstack.h"

struct AllocRecord;
struct FreeRecord;

struct AllocRecord
{
    uint64          address_;
    uint64          uid_;
    uint64          size_;
    uint            heapID_;
    Callstack       backtrace_;
    uint64          freeID_;

    bool operator == (const AllocRecord& rhs) const
    {
        return address_ == rhs.address_;
    }

    bool operator < (const AllocRecord& rhs) const
    {
        return address_ < rhs.address_;
    }
};

struct FreeRecord
{
    uint64          address_;
    uint64          uid_;
    uint            heapID_;
    Callstack       backtrace_;    
    uint64          allocID_;

    bool operator == (const FreeRecord& rhs) const
    {
        return address_ == rhs.address_;
    }

    bool operator < (const FreeRecord& rhs) const
    {
        return address_ < rhs.address_;
    }
};

#endif // RECORDS_H__