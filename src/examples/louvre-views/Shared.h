#ifndef SHARED_H
#define SHARED_H

class Compositor;
class Output;

#include <LNamespaces.h>

using namespace Louvre;

Compositor *comp();
std::list<Output*> &outps();
Int32 minimizedItemHeight();

#endif // SHARED_H
