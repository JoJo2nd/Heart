/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

/*
     This file need to be include-able outside of the normal heart include system.
     It contains common data type definitions that are used by debug game server messages. 
*/

#pragma once

#ifndef HNETDATASTRUCTS_H__
#define HNETDATASTRUCTS_H__

#include "base/hHeartConfig.h"
#include "base/hTypes.h"

namespace Heart
{

    struct hNetPacketHeader
    {
        static const hUint s_packetHeaderByteSize=8;
        enum typeID {
            eCommand = 1,
            eResult  = 2, 
            eReport  = 3,
        };
        hUint   type_   : 2;
        hUint   seqID_  : 30;
        hUint32 nameID_;
    };

}

#endif // HNETDATASTRUCTS_H__