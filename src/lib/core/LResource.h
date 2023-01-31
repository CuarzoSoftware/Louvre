#ifndef LRESOURCE_H
#define LRESOURCE_H

#include <LNamespaces.h>

class Louvre::LResource
{
public:
    LResource(wl_resource *resource);
    LResource(LCompositor *compositor, wl_client *client, const wl_interface *interface, Int32 version, UInt32 id, const void *implementation, wl_resource_destroy_func_t destroy);
    LResource(LClient *client, const wl_interface *interface, Int32 version, UInt32 id, const void *implementation, wl_resource_destroy_func_t destroy);
    ~LResource();

    LResource(const LResource&) = delete;
    LResource& operator= (const LResource&) = delete;

    wl_resource *resource() const;
    LClient *client() const;
    LCompositor *compositor() const;
    Int32 version() const;

    class LResourcePrivate;

    LResourcePrivate *imp() const;
private:
    LResourcePrivate *m_imp = nullptr;
};

#endif // LRESOURCE_H
