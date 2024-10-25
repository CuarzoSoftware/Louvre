#ifndef LTEST_H
#define LTEST_H

#include <string>
#include <LLog.h>
#include <LObject.h>

using namespace Louvre;

class LObjectTest : public LObject {};

void LSetTestName(const std::string &testName);
const std::string &LGetTestName();
void LAssert(const std::string &desc, bool condition);


#endif // LTEST_H
