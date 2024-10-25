#include <LTest.h>

static std::string _currentTestName { "MISSING TEST NAME "};

void LSetTestName(const std::string &testName)
{
    _currentTestName = testName;
}

const std::string &LGetTestName()
{
    return _currentTestName;
}

void LAssert(const std::string &desc, bool condition)
{
    if (condition)
        LLog::log("[\033[32mPASS\033[0m] %s : %s.", LGetTestName().c_str(), desc.c_str());
    else
    {
        LLog::log("[\033[31mFAIL\033[0m] %s : %s", LGetTestName().c_str(), desc.c_str());
        exit(EXIT_FAILURE);
    }
}
