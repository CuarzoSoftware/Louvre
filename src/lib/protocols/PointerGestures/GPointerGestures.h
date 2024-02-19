#ifndef GPOINTERGESTURES_H
#define GPOINTERGESTURES_H

#include <LResource.h>

class Louvre::Protocols::PointerGestures::GPointerGestures : public LResource
{
public:
    GPointerGestures(wl_client *client,
                     const wl_interface *interface,
                     Int32 version,
                     UInt32 id,
                     const void *implementation,
                     wl_resource_destroy_func_t destroy);
    LCLASS_NO_COPY(GPointerGestures)
    ~GPointerGestures();

    LPRIVATE_IMP_UNIQUE(GPointerGestures)
};

#endif // GPOINTERGESTURES_H
