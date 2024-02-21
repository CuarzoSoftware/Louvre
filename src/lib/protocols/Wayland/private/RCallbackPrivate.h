#ifndef RCALLBACKPRIVATE_H
#define RCALLBACKPRIVATE_H

#include <protocols/Wayland/RCallback.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(RCallback)
std::vector<RCallback*> *vec { nullptr };
};

#endif // RCALLBACKPRIVATE_H
