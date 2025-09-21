#include <LCompositor.h>
#include <LLauncher.h>
#include <LLog.h>
#include <LObject.h>
#include <unistd.h>

using namespace Louvre;

#include "LBitset_tests.h"
#include "LObject_test.h"
#include "LRegion_test.h"
#include "LWeak_test.h"

int main(int, char *[]) {
  LLog::init();

  LCompositor compositor;
  LObject_run_tests();
  LWeak_run_tests();
  LRegion_run_tests();
  LBitset_run_tests();

  return 0;
}
