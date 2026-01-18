#ifndef GPRIVATEHANDLEMANAGER_H
#define GPRIVATEHANDLEMANAGER_H

#include <CZ/Louvre/Roles/LBackgroundBlur.h>
#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZBitset.h>

class CZ::Protocols::PrivateHandle::GPrivateHandleManager final : public LResource
{
public:
    // Requests
    static void destroy(wl_client *client, wl_resource *resource);

    // Events
    void handle(const std::string &handle) noexcept;

private:
    LGLOBAL_INTERFACE
    GPrivateHandleManager(wl_client *client, Int32 version, UInt32 id);
    ~GPrivateHandleManager() noexcept;
};

#endif // GPRIVATEHANDLEMANAGER_H
