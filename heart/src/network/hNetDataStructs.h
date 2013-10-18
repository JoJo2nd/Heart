/********************************************************************

    filename:   hNetDataStructs.h  
    
    Copyright (c) 10:10:2013 James Moran
    
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