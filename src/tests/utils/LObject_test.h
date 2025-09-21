#ifndef LOBJECT_TEST_H
#define LOBJECT_TEST_H

#include <LObject.h>
#include <LTest.h>
#include <LWeak.h>

using namespace Louvre;

void LObject_test_01() {
  LSetTestName("LObject_test_01");
  LObjectTest obj;
  auto &weakRefs = LWeakUtils::objectRefs(&obj);
  LAssert("LObject weak refs count should be 0", weakRefs.size() == 0);
}

void LObject_test_02() {
  LSetTestName("LObject_test_02");
  LObjectTest obj;
  auto &weakRefs = LWeakUtils::objectRefs(&obj);

  {
    LWeak<LObjectTest> weak{&obj};
    LAssert("LObject weak refs count should be 1", weakRefs.size() == 1);
  }

  LAssert("LObject weak data counter should be 0", weakRefs.size() == 0);
}

void LObject_run_tests() {
  LObject_test_01();
  LObject_test_02();
}

#endif  // LOBJECT_TEST_H
