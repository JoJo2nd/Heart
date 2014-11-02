/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "testbed_precompiled.h"
#include "UnitTestFactory.h"


void UnitTestFactory::registerUnitTest(const hChar* name, IUnitTest* (*fn)(Heart::hHeartEngine*)) {
    UTest  t = {fn, name};
    tests_.push_back(t);
}

IUnitTest* UnitTestFactory::createUnitTest(hUint testidx) {
    for (const auto& i : tests_) {
        if (testidx == 0) {
            return i.func(engine_);
        }
        --testidx;
    }

    return nullptr;
}
