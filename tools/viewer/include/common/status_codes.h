/********************************************************************

    filename:   status_codes.h  
    
    Copyright (c) 31:12:2012 James Moran
    
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

#ifndef STATUS_CODES_H__
#define STATUS_CODES_H__

enum vErrorCode
{
    vOK_ALREADY_ADDED           = 1,
    vOK                         = 0,
    vERROR_ALREADY_CREATED      = -1,
    vERROR_CREATES_CYCLIC_REF   = -2,
    vERROR_HAS_DEPENDANTS       = -3,
    vERROR_NOT_FOUND            = -4,
    vERROR_DUPLICATE_RESORUCE_FOUND = -5,
    vERROR_DEP_PKG_NOT_FOUND    = -6,
    vERROR_DEP_ASSET_NOT_FOUND  = -7, 
    vERROR_FILE_IO              = -8,
};

#endif // STATUS_CODES_H__