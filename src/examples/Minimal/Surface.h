#ifndef SURFACE_H
#define SURFACE_H

#include <LSurface.h>

using namespace Louvre;

class Surface : public LSurface
{
public:
    Surface(Params *params, GLuint textureUnit = 1);
    virtual void mappingChanged() override;
    LRect prevRect;
    bool firstLaunch = true;
};

#endif // SURFACE_H
