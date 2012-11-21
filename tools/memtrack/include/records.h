/********************************************************************

	filename: 	records.h	
	
	Copyright (c) 10:11:2012 James Moran
	
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