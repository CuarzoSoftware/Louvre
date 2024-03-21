#ifndef GVIEWPORTER_H
#define GVIEWPORTER_H

#include <LResource.h>
#include <protocols/Viewporter/viewporter.h>

class Louvre::Protocols::Viewporter::GViewporter final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id) noexcept;
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void get_viewport(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept;

private:
    GViewporter(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GViewporter() noexcept;
};

#endif // GVIEWPORTER_H
