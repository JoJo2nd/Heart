/********************************************************************

	filename: 	JobManagerTest.h	
	
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
#ifndef JOBMANAGERTEST_H__
#define JOBMANAGERTEST_H__

#include "Heart.h"

class JobManagerTest : public Heart::hStateBase
{
public:
    JobManagerTest( Heart::HeartEngine* engine ) 
        : hStateBase( "ResourceLoadTest" )
        , engine_( engine )
        , jobChain1_(64)
        , jobChain2_(64)
    {

    }
    ~JobManagerTest() {}


    virtual hUint32				Main();		

private:

    Heart::HeartEngine*			engine_;
    Heart::hJobChain            jobChain1_;
    Heart::hJobChain            jobChain2_;
};

#endif // JOBMANAGERTEST_H__