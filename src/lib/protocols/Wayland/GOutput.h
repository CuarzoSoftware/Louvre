#ifndef OUTPUT_H
#define OUTPUT_H

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

    // Events
    void sendConfiguration();

    LPRIVATE_IMP(GOutput)
};

#endif // OUTPUT_H
