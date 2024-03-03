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
        LAssert("LWeak::count() should be 1", weak.count() == 1);
        LAssert("LWeak::get() should be != nullptr", weak.get() != nullptr);
        auto weaks(obj);
    }

    LAssert("LWeak::count() should be 0", weak.count() == 0);
    LAssert("LWeak::get() should be nullptr", weak.get() == nullptr);
}

void LWeak_test_04()
{
    LSetTestName("LWeak_test_04");

    LObjectTest obj;
    LWeak<LObjectTest> weak { &obj };

    {
        auto weak2 = weak;
        LAssert("LWeak::count() should be 2", weak.count() == 2);
    }

    LAssert("LWeak::count() should be 1", weak.count() == 1);
}

void LWeak_test_05()
{
    LSetTestName("LWeak_test_05");

    LObjectTest obj;
    LWeak<LObjectTest> weak { &obj };

    {
        LWeak<LObjectTest> weak2;
        weak2 = weak;
        LAssert("LWeak::count() should be 2", weak.count() == 2);
    }

    LAssert("LWeak::count() should be 1", weak.count() == 1);
}

void LWeak_test_06()
{
    LSetTestName("LWeak_test_06");

    LObjectTest obj;
    LWeak<LObjectTest> weak { &obj };
    auto weak2 = weak;

    LAssert("LWeak::count() should be 2", weak.count() == 2);
    weak.reset(&obj);
    LAssert("LWeak::count() should be 2", weak.count() == 2);
    LAssert("LWeak::count() 2 should be 2", weak2.count() == 2);
    weak.reset();
    LAssert("LWeak::count() should be 0", weak.count() == 0);
    LAssert("LWeak::count() 2 should be 1", weak2.count() == 1);
}

void LWeak_test_07()
{
    LSetTestName("LWeak_test_07");

    bool destroyed { false };
    LWeak<LObjectTest> weak;

    weak.setOnDestroyCallback([&destroyed](auto)
    {
        destroyed = true;
    });

    {
        LObjectTest obj;
        weak.reset(&obj);
    }

    LAssert("destroyed boolean should be true", destroyed == true);
}

void LWeak_test_08()
{
    LSetTestName("LWeak_test_08");

    LWeak<LObjectTest> weak[5];

    {
        LObjectTest obj;
        weak[2].reset(&obj);
        weak[0].reset(&obj);
        weak[1].reset(&obj);
        weak[4].reset(&obj);
        weak[3].reset(&obj);

        for (int i = 0; i < 4; i++)
            LAssert("Weak ref get() should NOT return nullptr", weak[i].get() != nullptr);
    }

    LAssert("Weak ref 3 count() should be 0", weak[2].count() == 0);

    for (int i = 0; i < 4; i++)
        LAssert("Weak ref get() should return nullptr", weak[i].get() == nullptr);
}

void LWeak_test_09()
{
    LSetTestName("LWeak_test_09");

    class Object : public LObject
    {
    public:
        ~Object()
        {
            notifyDestruction();
        }
    };

    Int32 notifyCount { 0 };

    LWeak<Object> weak;
    LWeak<Object> weak2;

    weak.setOnDestroyCallback([&notifyCount, &weak2](auto *obj)
    {
        notifyCount++;
        weak2.reset(obj);
        LAssert("Weak 2 get() should return nullptr", weak2.get() == nullptr);
    });

    {
        Object obj;
        weak.reset(&obj);
    }

    LAssert("Notify count should be 1", notifyCount == 1);
}

void LWeak_run_tests()
{
    LWeak_test_01();
    LWeak_test_02();
    LWeak_test_03();
    LWeak_test_04();
    LWeak_test_05();
    LWeak_test_06();
    LWeak_test_07();
    LWeak_test_08();
    LWeak_test_09();
}

#endif // LWEAK_TEST_H
