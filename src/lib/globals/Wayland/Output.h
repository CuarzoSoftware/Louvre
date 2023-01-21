#ifndef OUTPUT_H
#define OUTPUT_H

#include <LNamespaces.h>

class Louvre::Globals::Output
{
public:
    static void resource_destroy(wl_resource *resource);

#if LOUVRE_OUTPUT_VERSION >= 3
    static void release(wl_client *client,wl_resource *resource);
#endif

    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);

    // Louvre
    static void sendConfiguration(wl_resource *resource, LOutput *output);
};
#endif // OUTPUT_H
