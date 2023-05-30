#ifndef PRESENTATION_H
#define PRESENTATION_H

#include <LResource.h>

class Louvre::Protocols::WpPresentationTime::GWpPresentation : public LResource
{
public:
    GWpPresentation(wl_client *client,
                    const wl_interface *interface,
                    Int32 version,
                    UInt32 id,
                    const void *implementation,
                    wl_resource_destroy_func_t destroy);

    ~GWpPresentation();

    bool clock_id(UInt32 clockId) const;

    LPRIVATE_IMP(GWpPresentation)
};

#endif // PRESENTATION_H
