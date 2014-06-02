/********************************************************************

	filename: 	hRendererConstants.cpp	
	
	Copyright (c) 1:4:2012 James Moran
	
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

#include "base/hRendererConstants.h"
#include "base/hStringUtil.h"
#include <vector>

namespace Heart
{
    std::vector<std::string> hInputLayoutDesc::s_semanticNameMap;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hInputLayoutDesc::setSemanticName(const hChar* name) {
        for (hUint i=0, n=(hUint)s_semanticNameMap.size(); i!=n; ++i) {
            if (hStrCmp(s_semanticNameMap[i].c_str(), name) == 0) {
                semanticName_=i;
                return;
            }
        }
        semanticName_=(hUint)s_semanticNameMap.size();
        s_semanticNameMap.push_back(std::string(name));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hChar* hInputLayoutDesc::getSemanticName() const {
        return s_semanticNameMap[semanticName_].c_str();
    }

}