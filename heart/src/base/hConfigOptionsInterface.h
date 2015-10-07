/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

namespace Heart {
class hIConfigurationVariables {
public:
    virtual hUint getCVarUint(const hChar* key, hUint defval) = 0;
    virtual hInt getCVarInt(const hChar* key, hInt defval) = 0;
    virtual hFloat getCVarFloat(const hChar* key, hFloat defval) = 0;
    virtual hBool getCVarBool(const hChar* key, hBool defval) = 0;
    virtual const hChar* getCVarStr(const hChar* key, const hChar* defval) = 0;
};
}
