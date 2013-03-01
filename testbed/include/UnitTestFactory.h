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


#define UNIT_TEST_EXIT_CODE_RUNNING (1)
#define UNIT_TEST_EXIT_CODE_OK      (0)

    class IUnitTest
    {
    public:
        IUnitTest(Heart::hHeartEngine* engine) 
            : startExit_(hFalse)
            , exitCode_(UNIT_TEST_EXIT_CODE_RUNNING)
            , engine_(engine)
            , canRender_(hFalse)
        {

        }
        virtual ~IUnitTest() {}

        virtual void        BeginUnitTest() {};
        virtual hUint32     RunUnitTest() = 0;
        virtual void        RenderUnitTest() {};
        void                forceExitTest() { startExit_=hTrue; }
        hUint32             GetExitCode() const { return exitCode_; }
        hBool               GetCanRender() const { return canRender_; }
    protected:
        
        void                SetExitCode(hUint32 ecode) { exitCode_ = ecode; }
        void                SetCanRender(hBool val) { canRender_ = val; }
        hBool               getForceExitFlag() const { return startExit_; }

        Heart::hHeartEngine* engine_;

    private:

        hBool               startExit_;
        hUint32             exitCode_;
        hBool               canRender_;
    };

#define DECLARE_HEART_UNIT_TEST() public:\
    static const hChar* s_className_;\
    static IUnitTest* FactoryCreator(Heart::hHeartEngine*);\

#define DEFINE_HEART_UNIT_TEST(klass)\
    const hChar* klass::s_className_ = #klass ;\
    IUnitTest* klass::FactoryCreator(Heart::hHeartEngine* engine)\
    {\
        hcPrintf("Starting Test %s", s_className_);\
        return hNEW(Heart::GetGlobalHeap(), klass)(engine);\
    }\

#define REGISTER_UNIT_TEST(klass)\
    {klass::s_className_, &klass::FactoryCreator},

    typedef IUnitTest* (*UnitTestCreateFunc)(Heart::hHeartEngine*);

    struct UnitTestCreator
    {
        const hChar*        testName_;
        UnitTestCreateFunc  func_;
    };

    class UnitTestFactory 
    {
    public:
        UnitTestFactory(Heart::hHeartEngine* engine, UnitTestCreator* creators, hUint32 creatorCount)
            : creatorArray_(creators)
            , creatorCount_(creatorCount)
            , engine_(engine)
        {
            
        }

        IUnitTest* CreateUnitTest(const hChar* testName);

    private:

        const hUint32           creatorCount_;
        const UnitTestCreator*  creatorArray_;
        Heart::hHeartEngine*     engine_;
    };
#endif // UNITTESTFACTORY_H__