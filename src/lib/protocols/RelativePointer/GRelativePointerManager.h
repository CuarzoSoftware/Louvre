#ifndef GRELATIVEPOINTERMANAGER_H
#define GRELATIVEPOINTERMANAGER_H

#include <LResource.h>

class Louvre::Protocols::RelativePointer::GRelativePointerManager : public LResource
{
public:
    GRelativePointerManager(wl_client *client,
                     const wl_interface *interface,
                     Int32 version,
                     UInt32 id,
                     const void *implementation,
                     wl_resource_destroy_func_t destroy);
    ~GRelativePointerManager();

    LPRIVATE_IMP_UNIQUE(GRelativePointerManager)
};

#endif // GRELATIVEPOINTERMANAGER_H
