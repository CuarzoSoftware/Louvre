#include "LLog.h"
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <stdarg.h>
#include <mutex>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define BRELN "\n"

int level = 0;

using namespace Louvre;

void LLog::init()
{
    char *env = getenv("LOUVRE_DEBUG");

    if (env)
        level = atoi(env);
    else
        level = 0;
}


void LLog::fatal(const char *format, ...)
{
    if (level >= 1)
    {
        printf("%sLouvre fatal:%s ", KRED, KNRM);
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        printf(BRELN);
    }
}

void LLog::error(const char *format, ...)
{
    if (level >= 2)
    {
        printf("%sLouvre error:%s ", KRED, KNRM);
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        printf(BRELN);
    }
}

void LLog::warning(const char *format, ...)
{
    if (level >= 3)
    {
        printf("%sLouvre warning:%s ", KYEL, KNRM);
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        printf(BRELN);
    }
}

void LLog::debug(const char *format, ...)
{
    if (level >= 4)
    {
        printf("%sLouvre debug:%s ", KGRN, KNRM);
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        printf(BRELN);
    }
}

void LLog::log(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf(BRELN);
}

