#ifndef LRESOURCE_H
#define LRESOURCE_H

#include <LObject.h>

class Louvre::LResource : public LObject
{
public:
    LResource(wl_resource *resource);

    LResource(wl_client *client,
              const wl_interface *interface,
              Int32 version,
              UInt32 id,
              const void *implementation,
              wl_resource_destroy_func_t destroy);

    LResource(LClient *client,
              const wl_interface *interface,
              Int32 version,
              UInt32 id,
              const void *implementation,
              wl_resource_destroy_func_t destroy);

    ~LResource();

    LResource(const LResource&) = delete;
    LResource& operator= (const LResource&) = delete;

    wl_resource *resource() const;
    LClient *client() const;
    Int32 version() const;

    LPRIVATE_IMP(LResource)
};

#endif // LRESOURCE_H
