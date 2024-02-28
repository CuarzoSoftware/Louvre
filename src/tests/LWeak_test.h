#ifndef LWEAK_TEST_H
#define LWEAK_TEST_H

#include <LTest.h>
#include <LWeak.h>
#include <LObject.h>

using namespace Louvre;

void LWeak_test_01()
{
    LSetTestName("LWeak_test_01");
    LWeak<LObject> weak;
    LAssert("LWeak::count() should be 0", weak.count() == 0);
}

void LWeak_test_02()
{
    LSetTestName("LWeak_test_02");
    LWeak<LObject> weak;
    LAssert("LWeak::get() should be nullptr", weak.get() == nullptr);
}

void LWeak_test_03()
{
    LSetTestName("LWeak_test_03");

    LWeak<LObjectTest> weak;

    {
        LObjectTest obj;
        weak = obj.weakRef<LObjectTest>();
        LAssert("LWeak::count() should be 2", weak.count() == 2);
        LAssert("LWeak::get() should be != nullptr", weak.get() != nullptr);

        auto weaks(obj);
    }

    LAssert("LWeak::count() should be 1", weak.count() == 1);
    LAssert("LWeak::get() should be nullptr", weak.get() == nullptr);
}

void LWeak_test_04()
{
    LSetTestName("LWeak_test_04");

    LObjectTest obj;
    LWeak<LObjectTest> weak { &obj };

    {
        auto weak2 = weak;
        LAssert("LWeak::count() should be 3", weak.count() == 3);
    }

    LAssert("LWeak::count() should be 2", weak.count() == 2);
}

void LWeak_test_05()
{
    LSetTestName("LWeak_test_05");

    LObjectTest obj;
    LWeak<LObjectTest> weak { &obj };

    {
        LWeak<LObjectTest> weak2;
        weak2 = weak;
        LAssert("LWeak::count() should be 3", weak.count() == 3);
    }

    LAssert("LWeak::count() should be 2", weak.count() == 2);
}

void LWeak_test_06()
{
    LSetTestName("LWeak_test_06");

    LObjectTest obj;
    LWeak<LObjectTest> weak { &obj };
    auto weak2 = weak;

    LAssert("LWeak::count() should be 3", weak.count() == 3);
    weak.reset(&obj);
    LAssert("LWeak::count() should be 3", weak.count() == 3);
    LAssert("LWeak::count() 2 should be 3", weak2.count() == 3);
    weak.reset();
    LAssert("LWeak::count() should be 0", weak.count() == 0);
    LAssert("LWeak::count() 2 should be 2", weak2.count() == 2);
}

void LWeak_run_tests()
{
    LWeak_test_01();
    LWeak_test_02();
    LWeak_test_03();
    LWeak_test_04();
    LWeak_test_05();
    LWeak_test_06();
}

#endif // LWEAK_TEST_H
