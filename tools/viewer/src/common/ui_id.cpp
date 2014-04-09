/********************************************************************
    
    Copyright (c) 4:4:2014 James Moran
    
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

#include "precompiled/precompiled.h"
#include "common/ui_id.h"
#include <unordered_map>

namespace ui
{
    typedef std::unordered_map<std::string, ID> NameTable;

ID marshallNameToID(const char* name) {
    static size_t    nextNameID_ = 0;
    static NameTable nameMap_;

    std::string name_str(name);
    auto found_itr = nameMap_.find(name);
    if (found_itr != nameMap_.end()) {
        return found_itr->second;
    }

    ID ret_val = nextNameID_+wxID_HIGHEST+1;
    nameMap_.emplace(name_str, ret_val);
    ++nextNameID_;
    return ret_val;
}

}