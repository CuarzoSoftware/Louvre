#ifndef LOBJECT_TEST_H
#define LOBJECT_TEST_H

#include <LTest.h>
#include <LObject.h>
#include <LWeak.h>

using namespace Louvre;

void LObject_test_01()
{
    LSetTestName("LObject_test_01");
    LObjectTest obj;
    LWeakData *weakData = PrivateUtils::getObjectData(&obj);
    LAssert("LObject weak data counter should be 1", weakData->counter == 1);
    LAssert("LObject weak data isAlive should be true", weakData->isAlive);
}

void LObject_test_02()
{
    LSetTestName("LObject_test_02");
    LObjectTest obj;
    LWeakData *weakData = PrivateUtils::getObjectData(&obj);

    {
        LWeak<LObjectTest> weak(&obj);
        LAssert("LObject weak data counter should be 2", weakData->counter == 2);
    }

    LAssert("LObject weak data counter should be 1", weakData->counter == 1);
}

void LObject_test_03()
{
    LSetTestName("LObject_test_03");
    LWeakData *weakData;
    LWeak<LObjectTest> weak;

    {
        LObjectTest obj;
        weak.reset(&obj);
        weakData = PrivateUtils::getObjectData(&obj);
    }

    LAssert("LObject weak data isAlive should be false", weakData->isAlive == false);
}

void LObject_run_tests()
{
    LObject_test_01();
    LObject_test_02();
    LObject_test_03();
}

#endif // LOBJECT_TEST_H
