#ifndef LRESOURCEPRIVATE_H
#define LRESOURCEPRIVATE_H

#include <LResource.h>

using namespace Louvre;

class LResource::LResourcePrivate
{
public:
    LClient *client = nullptr;
    wl_resource *resource = nullptr;
};

#endif // LRESOURCEPRIVATE_H
