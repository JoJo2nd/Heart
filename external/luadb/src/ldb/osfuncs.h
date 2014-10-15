/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#ifndef OSFUNCS_H__
#define OSFUNCS_H__

#include "types.h"

void ldbOSSleep(float secs);
ldb_uint32 ldbOSGetSysTimems();

#endif // OSFUNCS_H__