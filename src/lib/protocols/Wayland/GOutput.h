#ifndef GOUTPUT_H
#define GOUTPUT_H

#include <LResource.h>

class Louvre::Protocols::Wayland::GOutput : public LResource
{
public:
    GOutput(LOutput *output,
            LClient *lClient,
            const wl_interface *interface,
            Int32 version,
            UInt32 id,
            const void *implementation,
            wl_resource_destroy_func_t destroy);

    ~GOutput();

    LOutput *output() const;

    // Send all events
    void sendConfiguration();

    // Since 1
    bool geometry(Int32 x, Int32 y,
                  Int32 physicalWidth, Int32 physicalHeight,
                  Int32 subpixel, const char *make,
                  const char *model, Int32 transform);
    bool mode(UInt32 flags, Int32 width, Int32 height, Int32 refresh);

    // Since 2
    bool done();
    bool scale(Int32 factor);

    // Since 4
    bool name(const char *name);
    bool description(const char *description);

    LPRIVATE_IMP(GOutput)
};

#endif // GOUTPUT_H
