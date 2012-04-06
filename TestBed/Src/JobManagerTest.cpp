/********************************************************************

	filename: 	JobManagerTest.cpp	
	
	Copyright (c) 19:2:2012 James Moran
	
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

#include "JobManagerTest.h"


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

hUint32 JobManagerTest::Main()
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

    return Heart::hStateBase::CONTINUE;
}
