/********************************************************************

	filename: 	UnitTestFactory.cpp	
	
	Copyright (c) 24:9:2012 James Moran
	
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
