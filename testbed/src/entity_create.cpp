/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#include "base/hTypes.h"
#include "base/hUUID.h"
#include "components/hObjectFactory.h"
#include "components/hEntity.h"
#include "components/hEntityFactory.h"
#include "testbed.pb.h"
#include "testbed.pb.diff.h"
#include "UnitTestFactory.h"

namespace Heart {
class hHeartEngine;
}

struct TestGameComponent1 : public Heart::hEntityComponent {
    hObjectType(TestGameComponent1, Testbed::proto::TestGameComponent1);
    Heart::hString entryName;
    hUint64 entryOffset;
    hUint64 entrySize;
    Heart::hString entryType;
};
hRegisterComponentObjectType(TestGameComponent1, TestGameComponent1, Testbed::proto::TestGameComponent1);
hBool TestGameComponent1::serialiseObject(Testbed::proto::TestGameComponent1* obj) const {
    obj->set_entryname(entryName);
    obj->set_entryoffset(entryOffset);
    obj->set_entrysize(entrySize);
    obj->set_entrytype(entryType);
    return hTrue;
}
hBool TestGameComponent1::deserialiseObject(Testbed::proto::TestGameComponent1* obj) {
    if (obj->has_entryname()) {
        entryName = obj->entryname();
    }
    if (obj->has_entryoffset()) {
        entryOffset = obj->entryoffset();
    }
    if (obj->has_entrysize()) {
        entrySize = obj->entrysize();
    }
    if (obj->has_entrytype()) {
        entryType = obj->entrytype();
    }
    return hTrue;
}
hBool TestGameComponent1::linkObject() {
    return hTrue;
}

struct TestGameComponent2 : public Heart::hEntityComponent {
    hObjectType(TestGameComponent2, Testbed::proto::TestGameComponent2);
    Heart::hString packageDependencies;
    hUint64 highscore;
};
hRegisterComponentObjectType(TestGameComponent2, TestGameComponent2, Testbed::proto::TestGameComponent2);
hBool TestGameComponent2::serialiseObject(Testbed::proto::TestGameComponent2* obj) const {
    obj->set_highscore(highscore);
    return hTrue;
}
hBool TestGameComponent2::deserialiseObject(Testbed::proto::TestGameComponent2* obj) {
    if (obj->has_highscore()) {
        highscore = obj->highscore();
    }
    return hTrue;
}
hBool TestGameComponent2::linkObject() {
    return hTrue;
}

class EntityTest : public IUnitTest {
public:
    EntityTest( Heart::hHeartEngine* engine ) 
        : IUnitTest( engine )
        , doTest_(hTrue)
        , esureNotDeadstripped(TestGameComponent1::auto_object_registered && TestGameComponent2::auto_object_registered)
    {
    }
    ~EntityTest() {}

    virtual hUint32 RunUnitTest() override {
        using namespace Heart;
        if (doTest_) {
            //Test diff'ing & merging entity proto buffs
            Testbed::proto::TestGameComponent1 a1, a2, a3, a4;
            a1.set_entryname("test_game_component_1");
            a1.set_entryoffset(1024);
            a1.set_entrysize(4096);
            a1.set_entrytype("test_type_1");

            a2.CopyFrom(a1);
            a2.set_entryname("test_game_component_1"); // Set to same value from a1, shouldn't appear in a3
            //a2.set_entryoffset(1024); // Don't set to inherit value from a1
            a2.set_entrysize(9001);
            a2.set_entrytype("some_different_test_type"); //

            Testbed::proto::TestGameComponent1_diff(a1, a2, a3);
            hcAssert(!a3.has_entryname());
            hcAssert(!a3.has_entryoffset());
            hcAssert(a3.entrysize() == 9001);
            hcAssert(a3.entrytype() == "some_different_test_type");

            Testbed::proto::TestGameComponent1_merge(a1, a3, a4);
            hcAssert(a2.entryname() == a4.entryname());
            hcAssert(a2.entryoffset() == a4.entryoffset());
            hcAssert(a2.entrysize() == a4.entrysize());
            hcAssert(a2.entrytype() == a4.entrytype());

            // Test creating entities
            hEntityFactory::hEntityCreateDesc entities[] = {
                {hUUID::generateUUID(), "TestGameObjectDef"},
                {hUUID::generateUUID(), "TestGameObjectDef2"},
            };
            hEntityFactory::createEntityContext("test_entity_context", entities, hStaticArraySize(entities));

            // Test Save & Load
            doTest_=hFalse;
        }

        //SetExitCode(UNIT_TEST_EXIT_CODE_OK);
        return 0;
    }


private:
    hBool doTest_;
    hBool esureNotDeadstripped;
};

DEFINE_HEART_UNIT_TEST(EntityTest);