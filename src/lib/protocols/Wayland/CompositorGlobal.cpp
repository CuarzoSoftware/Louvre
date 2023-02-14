#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>
#include <private/LSurfacePrivate.h>

#include <protocols/Wayland/private/CompositorGlobalPrivate.h>
#include <LLog.h>

using namespace Louvre::Protocols::Wayland;

struct wl_compositor_interface compositor_implementation =
{
    .create_surface = &CompositorGlobal::CompositorGlobalPrivate::create_surface,
    .create_region = &CompositorGlobal::CompositorGlobalPrivate::create_region
};

CompositorGlobal::CompositorGlobal(
        LClient *client,
        const wl_interface *interface,
        Int32 version,
        UInt32 id,
        const void *implementation,
        wl_resource_destroy_func_t destroy) : LResource(
                                                  client,
                                                  interface,
                                                  version,
                                                  id,
                                                  implementation,
                                                  destroy)
{
    m_imp = new CompositorGlobalPrivate();
    client->imp()->compositorGlobal = this;
}

CompositorGlobal::~CompositorGlobal()
{
    delete m_imp;
    client()->imp()->compositorGlobal = nullptr;
}

void CompositorGlobal::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    LCompositor *lCompositor = (LCompositor*)data;

    LClient *lClient = lCompositor->getClientFromNativeResource(client);

    if(lClient->compositorGlobal())
    {
        LLog::warning("Client bound twice to the wl_compositor singleton global. Ignoring it.");
        return;
    }

    new CompositorGlobal(lClient,
                         &wl_compositor_interface,
                         version,
                         id,
                         &compositor_implementation,
                         &CompositorGlobal::CompositorGlobalPrivate::resource_destroy);
}

CompositorGlobal::CompositorGlobalPrivate *CompositorGlobal::imp() const
{
    return m_imp;
}


