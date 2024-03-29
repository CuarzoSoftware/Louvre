#ifndef LREGION_TEST_H
#define LREGION_TEST_H

#include <LTest.h>
#include <LRegion.h>

using namespace Louvre;

void LRegion_test_01()
{
    LSetTestName("LRegion_test_01");

    LRegion region;
    LPointF testPoint { 30.f, 30.f };
    LPointF closestPoint = region.closestPointFrom(testPoint);
    LAssert("closestPoint should be (30, 30)", closestPoint == testPoint);

    region.addRect(5, 10, 50, 100);
    closestPoint = region.closestPointFrom(testPoint);
    LAssert("closestPoint should be (30, 30)", closestPoint == testPoint);

    testPoint = LPointF(0.f, 0.f);
    closestPoint = region.closestPointFrom(testPoint);
    LAssert("closestPoint should be (5, 10)", closestPoint == LPointF(5, 10));

    testPoint = LPointF(0.f, 15.f);
    closestPoint = region.closestPointFrom(testPoint);
    LAssert("closestPoint should be (5, 15)", closestPoint == LPointF(5, 15));

    testPoint = LPointF(400.f, 15.f);
    closestPoint = region.closestPointFrom(testPoint);
    LAssert("closestPoint should be (50, 15)", closestPoint == LPointF(55, 15));

    region.addRect(150, 0, 10, 10);
    closestPoint = region.closestPointFrom(testPoint);
    LAssert("closestPoint should be (160, 10)", closestPoint == LPointF(160, 10));
}

void LRegion_run_tests()
{
    LRegion_test_01();
}

#endif // LREGION_TEST_H
