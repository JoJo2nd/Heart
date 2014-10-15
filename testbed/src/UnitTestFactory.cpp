/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "testbed_precompiled.h"
#include "UnitTestFactory.h"


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

IUnitTest* UnitTestFactory::CreateUnitTest( const hChar* testName )
{   
    if (!testName)
    {
        return NULL;
    }

    for (hUint32 i = 0; i < creatorCount_; ++i)
    {
        if (Heart::hStrICmp(testName, creatorArray_[i].testName_) == 0)
        {
            return creatorArray_[i].func_(engine_);
        }
    }

    return NULL;
}
