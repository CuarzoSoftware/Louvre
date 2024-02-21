#ifndef GVIEWPORTER_H
#define GVIEWPORTER_H

#include <LResource.h>

class Louvre::Protocols::Viewporter::GViewporter : public LResource
{
public:
    GViewporter(LClient *client,
                const wl_interface *interface,
                Int32 version,
                UInt32 id,
                const void *implementation);
    ~GViewporter();

    LPRIVATE_IMP_UNIQUE(GViewporter)
};

#endif // GVIEWPORTER_H
