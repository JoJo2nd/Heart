/********************************************************************

	filename: 	UnitTestFactory.h	
	
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
#pragma once

#ifndef UNITTESTFACTORY_H__
#define UNITTESTFACTORY_H__

#include "Heart.h"

#define UNIT_TEST_EXIT_CODE_RUNNING (1)
#define UNIT_TEST_EXIT_CODE_OK      (0)

    class IUnitTest
    {
    public:
        IUnitTest(Heart::HeartEngine* engine) 
            : exitCode_(UNIT_TEST_EXIT_CODE_RUNNING)
            , engine_(engine)
        {

        }
        virtual ~IUnitTest() {}

        virtual void        BeginUnitTest() {};
        virtual hUint32     RunUnitTest() = 0;
        virtual void        ForceExitTest() {};
        hUint32             GetExitCode() { return exitCode_; }
    protected:
        
        void                SetExitCode(hUint32 ecode) { exitCode_ = ecode; }

        Heart::HeartEngine* engine_;

    private:

        hUint32             exitCode_;
    };

#define DECLARE_HEART_UNIT_TEST() public:\
    static const hChar* s_className_;\
    static IUnitTest* FactoryCreator(Heart::HeartEngine*);\

#define DEFINE_HEART_UNIT_TEST(klass)\
    const hChar* klass::s_className_ = #klass ;\
    IUnitTest* klass::FactoryCreator(Heart::HeartEngine* engine)\
    {\
        return hNEW(Heart::GetGlobalHeap(), klass)(engine);\
    }\

#define REGISTER_UNIT_TEST(klass)\
    {klass::s_className_, &klass::FactoryCreator},

    typedef IUnitTest* (*UnitTestCreateFunc)(Heart::HeartEngine*);

    struct UnitTestCreator
    {
        const hChar*        testName_;
        UnitTestCreateFunc  func_;
    };

    class UnitTestFactory 
    {
    public:
        UnitTestFactory(Heart::HeartEngine* engine, UnitTestCreator* creators, hUint32 creatorCount)
            : creatorArray_(creators)
            , creatorCount_(creatorCount)
            , engine_(engine)
        {
            
        }

        IUnitTest* CreateUnitTest(const hChar* testName);

    private:

        const hUint32           creatorCount_;
        const UnitTestCreator*  creatorArray_;
        Heart::HeartEngine*     engine_;
    };
#endif // UNITTESTFACTORY_H__