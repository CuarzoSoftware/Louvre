#ifndef RCALLBACKPRIVATE_H
#define RCALLBACKPRIVATE_H

#include <protocols/Wayland/RCallback.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(RCallback)
static void resource_destroy(wl_resource *resource);
std::list<RCallback*>*list = nullptr;
std::list<RCallback*>::iterator listLink;
};

#endif // RCALLBACKPRIVATE_H
