/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef JOBMANAGERTEST_H__
#define JOBMANAGERTEST_H__

#include "UnitTestFactory.h"

class JobManagerTest : public IUnitTest
{
    DECLARE_HEART_UNIT_TEST();
public:
    JobManagerTest( Heart::hHeartEngine* engine ) 
        : IUnitTest(engine)
        , jobChain1_(64)
        , jobChain2_(64)
    {

    }
    ~JobManagerTest() {}

    virtual hUint32				RunUnitTest();		

private:

    Heart::hJobChain            jobChain1_;
    Heart::hJobChain            jobChain2_;
};

#endif // JOBMANAGERTEST_H__