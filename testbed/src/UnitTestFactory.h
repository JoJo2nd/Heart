/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include <vector>

namespace Heart {
class hHeartEngine;
namespace hRenderer {
struct hCmdList;
}
}

#define UNIT_TEST_EXIT_CODE_RUNNING (1)
#define UNIT_TEST_EXIT_CODE_OK      (0)

    class IUnitTest {
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
        virtual Heart::hRenderer::hCmdList* RenderUnitTest() { return nullptr; };
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

#define DEFINE_HEART_UNIT_TEST(klass)\
    IUnitTest* autogen_FactoryCreator__##klass (Heart::hHeartEngine* engine) {\
        return new klass(engine);\
    }\

#define DEFINE_HEART_UNIT_TEST_INLINE(klass) hFORCEINLINE DEFINE_HEART_UNIT_TEST(klass)

#define REGISTER_UNIT_TEST(factory, klass) \
    extern IUnitTest* autogen_FactoryCreator__##klass (Heart::hHeartEngine* engine); \
    (factory).registerUnitTest(#klass, &autogen_FactoryCreator__##klass )

    typedef IUnitTest* (*UnitTestCreateFunc)(Heart::hHeartEngine*);

    struct UnitTestCreator {
        const hChar*        testName_;
        UnitTestCreateFunc  func_;
    };

    class UnitTestFactory {
    public:
        UnitTestFactory(Heart::hHeartEngine* engine)
            : engine_(engine) {
            
        }

        void        registerUnitTest(const hChar* name, IUnitTest* (*)(Heart::hHeartEngine*));
        IUnitTest*  createUnitTest(hUint testindex);
        const char* getUnitTestName(hUint testindex);
        hSize_t     getTestCount() const { return tests_.size(); }

    private:

        struct UTest {
            IUnitTest* (*func)(Heart::hHeartEngine*);
            const hChar* name;
        };

        std::vector<UTest>   tests_;
        Heart::hHeartEngine* engine_;
    };