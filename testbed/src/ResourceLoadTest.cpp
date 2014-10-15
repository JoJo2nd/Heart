/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "testbed_precompiled.h"
#include "ResourceLoadTest.h"

DEFINE_HEART_UNIT_TEST(ResourceLoadTest);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 ResourceLoadTest::RunUnitTest()
{
    switch(state_)
    {
    case eBeginLoad:
        {
            hcPrintf("Loading package \"UNITTEST\"");
            engine_->GetResourceManager()->mtLoadPackage("UNITTEST");
            state_ = eLoading;
        }
        break;
    case eLoading:
        {
            if (engine_->GetResourceManager()->mtIsPackageLoaded("UNITTEST"))
            {
                hcPrintf("Loaded package \"UNITTEST\"");
                state_ = eWait;
                timer_ = 0.f;
            }
        }
        break;
    case eWait:
        {
            timer_ += Heart::hClock::Delta();
            if (getForceExitFlag()) {
                state_ = eBeginUnload;
            }
        }
        break;
    case eBeginUnload:
        {
            engine_->GetResourceManager()->mtUnloadPackage("UNITTEST");
            hcPrintf("Unloading package \"UNITTEST\"");
            state_ = eExit;
        }
        break;
    case eExit:
        {
            hcPrintf("End unit test resource package load test.");
            SetExitCode(UNIT_TEST_EXIT_CODE_OK);
        }
        break;
    }

    return 0;
}
