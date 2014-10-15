/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "testbed_precompiled.h"
#include "JobManagerTest.h"

DEFINE_HEART_UNIT_TEST(JobManagerTest);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static void jobFunc1( const Heart::hJobDesc& jobdesc )
{
    hcPrintf( "jobFunc1 %d", *((hUint32*)jobdesc.inputBuffes_[0].ptr_) );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static void jobFunc2( const Heart::hJobDesc& jobdesc )
{
    hcPrintf( "jobFunc2 %d", *((hUint32*)jobdesc.inputBuffes_[0].ptr_) );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 JobManagerTest::RunUnitTest()
{
    hUint32 tag;
    hUint32 nums[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
 
    tag = 0;
    for ( hUint32 i = 0; i < 10; ++i )
    {
        Heart::hJobDesc desc;
        Heart::hZeroMem( &desc, sizeof(desc) );
        desc.AppendInputBuffer( &nums[i], sizeof(nums[i]) );
        desc.scratchSize_ = 0;
        jobChain1_.PushJob( desc, jobFunc1, tag );
        ++tag;
    }

    tag = 0;
    for ( hUint32 i = 0; i < 15; ++i )
    {
        Heart::hJobDesc desc;
        Heart::hZeroMem( &desc, sizeof(desc) );
        desc.AppendInputBuffer( &nums[i], sizeof(nums[i]) );
        desc.scratchSize_ = 0;
        jobChain2_.PushJob( desc, jobFunc2, 0 );
        if ( i % 4 == 0 ) ++tag;
    }

    engine_->GetJobManager()->PushJobChain( &jobChain1_ );
    engine_->GetJobManager()->PushJobChain( &jobChain2_ );

    jobChain1_.Sync();
    jobChain2_.Sync();

    hcPrintf(__FUNCTION__" Test Complete");
    SetExitCode(UNIT_TEST_EXIT_CODE_OK);

    return 0;
}
