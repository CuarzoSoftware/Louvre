#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <LNamespaces.h>

class Louvre::Globals::Keyboard
{
public:
    static void resource_destroy(wl_resource *resource);

#if LOUVRE_SEAT_VERSION >= 3
    static void release(wl_client *client, wl_resource *resource);
#endif
};

#endif // KEYBOARD_H
