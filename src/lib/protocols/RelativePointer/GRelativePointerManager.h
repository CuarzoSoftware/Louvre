#ifndef GRELATIVEPOINTERMANAGER_H
#define GRELATIVEPOINTERMANAGER_H

#include <LResource.h>
#include <protocols/RelativePointer/relative-pointer-unstable-v1.h>

class Louvre::Protocols::RelativePointer::GRelativePointerManager final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id) noexcept;
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void get_relative_pointer(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *pointer) noexcept;

private:
    GRelativePointerManager(wl_client *client,
                     Int32 version,
                     UInt32 id) noexcept;
    ~GRelativePointerManager() noexcept;
};

#endif // GRELATIVEPOINTERMANAGER_H
