#ifndef GRELATIVEPOINTERMANAGER_H
#define GRELATIVEPOINTERMANAGER_H

#include <CZ/Louvre/LResource.h>

class CZ::Protocols::RelativePointer::GRelativePointerManager final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void get_relative_pointer(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *pointer) noexcept;
private:
    LGLOBAL_INTERFACE
    GRelativePointerManager(wl_client *client, Int32 version, UInt32 id);
    ~GRelativePointerManager() noexcept;
};

#endif // GRELATIVEPOINTERMANAGER_H
