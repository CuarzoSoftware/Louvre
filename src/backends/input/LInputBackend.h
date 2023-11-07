#ifndef LINPUTBACKEND
#define LINPUTBACKEND

#include <LNamespaces.h>

class Louvre::LInputBackend
{
public:

    // Backend Identifier
    static UInt32 id();

    // Initialize the backend
    static bool initialize();

    // Get capabilities (pointer, keyboard, touch)
    static UInt32 getCapabilities();

    // Get context handle
    static void *getContextHandle();

    // Suspend events when switching TTY
    static void suspend();

    // Force processing of pending events
    static void forceUpdate();

    // Resume events upon returning to the TTY
    static void resume();

    // Deinitialize the backend
    static void uninitialize();
};

#endif // LINPUTBACKEND
