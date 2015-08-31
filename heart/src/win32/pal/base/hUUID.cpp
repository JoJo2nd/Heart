/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hUUID.h"
#include "base/hMemoryUtil.h"
#include "base/hStringUtil.h"
#include <windows.h>


bool hUuid_t::operator==(const hUuid_t& rhs) const {
    return Heart::hUUID::compareUUID(*this, rhs);
}

namespace Heart {
namespace hUUID {
    static_assert(sizeof(hUuid_t) >= sizeof(GUID), "hUuid_t must provide enough space to store a GUID object");

    HEART_EXPORT hUuid_t HEART_API generateUUID() {
        hUuid_t out_uuid;
        hZeroMem(&out_uuid, sizeof(hUuid_t));
        CoCreateGuid((LPGUID)&out_uuid);
        return out_uuid;
    }
    HEART_EXPORT hBool HEART_API compareUUID(const hUuid_t& lhs, const hUuid_t& rhs) {
        return hMemCmp((void*)&lhs, (void*)&rhs, sizeof(GUID)) == 0;
    }
    HEART_EXPORT hSize_t HEART_API toStringSize(const hUuid_t& a) {
        return 37;
    }
    HEART_EXPORT hSize_t HEART_API toString(const hUuid_t& a, hChar* out, hSize_t out_size) {
        hcAssertMsg(out_size >= 37, "UUID to string buffer is too small");
        auto* bytes = (const char*)&a;
        auto* ptr = out;
        for (auto i = 0u; i < 36;) {
            if (i != 8 && i != 13 && i != 18 && i != 23) {
                int val = (*bytes)&0xFF;
                if (hStrPrintf(ptr, (hUint32)out_size-i, "%02x", val) != 2) {
                    return 0;
                }
                ++bytes;
                ptr+=2;
                i+=2;
            } else {
                *ptr = '-';
                ++ptr;
                ++i;
            }
        }
        return 36;
    }
    HEART_EXPORT hUuid_t HEART_API fromString(const hChar* out, hSize_t in_size) {
        hUuid_t out_uuid;
        hZeroMem(&out_uuid, sizeof(out_uuid));
        auto len = hStrLen(out);
        for (auto i=0u, dst=0u; i < in_size && i < len; ) {
            if (out[i] == '-') {
                ++i;
                continue;
            } 
            auto* bytes = ((char*)&out_uuid)+dst;
            *bytes = 0;
            for (auto j=0; j < 2; ++j) {
                auto radix = (int)pow(16,1-j);
                if (out[i+j] >= '0' && out[i+j] <= '9') {
                    *bytes += (out[i+j] - '0')*radix;
                } else if (out[i+j] >= 'A' && out[i+j] <= 'F') {
                    *bytes += (out[i+j] - 'A' + 10)*radix;
                } else if (out[i + j] >= 'a' && out[i + j] <= 'f') {
                    *bytes += (out[i + j] - 'a' + 10)*radix;
                } else {
                    hcAssertFailMsg("Unknown UUID string character %c", out[i+j]);
                    hZeroMem(&out_uuid, sizeof(out_uuid));
                    return out_uuid;
                }
            }
            i+=2;
            ++dst;
        }
        return out_uuid;
    }
    HEART_EXPORT hBool HEART_API isNull(const hUuid_t& a) {
        hUuid_t zero;
        hZeroMem(&zero, sizeof(zero));
        return compareUUID(a, zero);
    }

	HEART_EXPORT hUuid_t HEART_API getInvalid() {
		hUuid_t zero = {0};
		return zero;
	}
}
}