/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#include "base/hTypes.h"
#include "base/hUUID.h"
#include "UnitTestFactory.h"

namespace Heart {
class hHeartEngine;
}

class UUIDTests : public IUnitTest {
public:
    UUIDTests( Heart::hHeartEngine* engine ) 
        : IUnitTest( engine )
        , doTest_(hTrue)
    {
    }
    ~UUIDTests() {}

    virtual hUint32 RunUnitTest() override {
        using namespace Heart;
        if (doTest_) {
            hcPrintf("%s Generating Data", __FUNCTION__);

            auto uuid1 = hUUID::generateUUID();
            auto uuid2 = hUUID::generateUUID();
            auto uuid3 = hUUID::generateUUID();
            auto uuid4 = hUUID::generateUUID();

            hcPrintf("%s Converting Data", __FUNCTION__);

            hChar uuid1str[37];
            hChar uuid2str[37];
            hChar uuid3str[37];
            hChar uuid4str[37];
            hUUID::toString(uuid1, uuid1str, sizeof(uuid1str));
            hcPrintf("UUID 1 is \"%s\"", uuid1str);
            hUUID::toString(uuid2, uuid2str, sizeof(uuid2str));
            hcPrintf("UUID 2 is \"%s\"", uuid2str);
            hUUID::toString(uuid3, uuid3str, sizeof(uuid3str));
            hcPrintf("UUID 3 is \"%s\"", uuid3str);
            hUUID::toString(uuid4, uuid4str, sizeof(uuid4str));
            hcPrintf("UUID 4 is \"%s\"", uuid4str);

            hcPrintf("%s Decoding Data",__FUNCTION__);

            hUuid_t uuid5 = hUUID::fromString(uuid1str, sizeof(uuid1str));
            hUuid_t uuid6 = hUUID::fromString(uuid2str, sizeof(uuid2str));
            hUuid_t uuid7 = hUUID::fromString(uuid3str, sizeof(uuid3str));
            hUuid_t uuid8 = hUUID::fromString(uuid4str, sizeof(uuid4str));

            hcPrintf("%s Comparing Data",__FUNCTION__);

            hcPrintf("uuid1 == uuid5 %s", hUUID::compareUUID(uuid1, uuid5) ? "Success" : "Failed");
            hcPrintf("uuid2 == uuid6 %s", hUUID::compareUUID(uuid2, uuid6) ? "Success" : "Failed");
            hcPrintf("uuid3 == uuid7 %s", hUUID::compareUUID(uuid3, uuid7) ? "Success" : "Failed");
            hcPrintf("uuid4 == uuid8 %s", hUUID::compareUUID(uuid4, uuid8) ? "Success" : "Failed");

            hcPrintf("%s Test Complete",__FUNCTION__);

            doTest_=hFalse;
        }

        {
            SetExitCode(UNIT_TEST_EXIT_CODE_OK);
        }

        return 0;
    }

    void GenRandomNumbers(hUint* t, hUint tc) 
    {
        for ( hUint i = 0; i < tc; ++i )
        {
            t[i] = rand();
        }
    }


private:
    hBool          doTest_;
};

DEFINE_HEART_UNIT_TEST(UUIDTests);