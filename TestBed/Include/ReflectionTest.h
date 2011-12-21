/********************************************************************

	filename: 	ReflectionTest.h	
	
	Copyright (c) 26:9:2011 James Moran
	
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
#ifndef REFLECTIONTEST_H__
#define REFLECTIONTEST_H__

#include "Heart.h"
#include "TestReflectionWriter.h"

class ReflectionTest : public Heart::hStateBase
{
public:
    ReflectionTest( Heart::HeartEngine* engine ) 
        : hStateBase( "ReflectionTest" )
        , engine_( engine )
    {

    }
    ~ReflectionTest() {}

    virtual void				PreEnter() {}
    virtual hUint32				Enter() { return Heart::hStateBase::FINISHED; }
    virtual void				PostEnter() {}
    virtual hUint32				Main();
    virtual void				MainRender() {}
    virtual void				PreLeave() {}
    virtual hUint32				Leave() { return Heart::hStateBase::FINISHED; }

private:

    Heart::HeartEngine*						engine_;
};

#endif // REFLECTIONTEST_H__