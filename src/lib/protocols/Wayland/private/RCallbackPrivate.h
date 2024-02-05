#ifndef RCALLBACKPRIVATE_H
#define RCALLBACKPRIVATE_H

#include <protocols/Wayland/RCallback.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(RCallback)
static void resource_destroy(wl_resource *resource);
std::vector<RCallback*> *vec { nullptr };
};

#endif // RCALLBACKPRIVATE_H
