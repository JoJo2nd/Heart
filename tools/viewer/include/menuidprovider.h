/********************************************************************

    filename:   menuidprovider.h  
    
    Copyright (c) 4:1:2013 James Moran
    
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

#ifndef MENUIDPROVIDER_H__
#define MENUIDPROVIDER_H__

class MenuIDProvider : public vMenuIDProvider
{
public:
    unsigned int aquireMenuID(unsigned int id)
    {
        if (allocatedIDs_.size() <= id) {
            allocatedIDs_.resize(id+1);
        }
        allocatedIDs_[id]=getUniqueMenuID();
        return allocatedIDs_[id];
    }
    unsigned int getMenuID(unsigned int id)
    {
        return allocatedIDs_[id];
    }

private:

    static uint getUniqueMenuID()
    {
        static uint s_currentID=0;
        ++s_currentID;
        return cuiID_MAX+s_currentID;
    }

    typedef std::vector< uint > IDLookArrayType;

    IDLookArrayType allocatedIDs_;
};

#endif // MENUIDPROVIDER_H__