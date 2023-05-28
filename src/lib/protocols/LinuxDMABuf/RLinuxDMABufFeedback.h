#ifdef nn

#ifndef FEEDBACK_H
#define FEEDBACK_H

#include <LNamespaces.h>

class Louvre::Extensions::LinuxDMABuffer::Feedback
{
public:
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
};

#endif // FEEDBACK_H
#endif
