/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef RESOURCELOADTEST_H__
#define RESOURCELOADTEST_H__

#include "UnitTestFactory.h"

class ResourceLoadTest : public IUnitTest
{
    DECLARE_HEART_UNIT_TEST();
public:
    ResourceLoadTest(Heart::hHeartEngine* engine) 
        : IUnitTest( engine )
        , state_(eBeginLoad)
    {
    }
    ~ResourceLoadTest() {}

    hUint32				RunUnitTest();

private:

    enum State
    {
        eBeginLoad,
        eLoading,
        eWait,
        eBeginUnload,
        eExit,
    };

    State       state_;
    hFloat      timer_;
};

#endif // RESOURCELOADTEST_H__
