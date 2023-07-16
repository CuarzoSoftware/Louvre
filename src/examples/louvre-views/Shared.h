#ifndef SHARED_H
#define SHARED_H

class Compositor;
class Output;

#include <LNamespaces.h>

using namespace Louvre;

Compositor *comp();
std::list<Output*> &outps();
Int32 minimizedItemHeight();
void arrangeOutputs();

struct SharedData
{
    LTexture *circleTextureTL, *circleTextureTR, *circleTextureBR, *circleTextureBL;
};

SharedData *shared();

#endif // SHARED_H
