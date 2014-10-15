/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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