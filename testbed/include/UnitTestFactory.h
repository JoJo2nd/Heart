/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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
        return new klass(engine);\
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