/********************************************************************

	filename: 	ResourceLoadTest.cpp	
	
	Copyright (c) 3:12:2011 James Moran
	
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
