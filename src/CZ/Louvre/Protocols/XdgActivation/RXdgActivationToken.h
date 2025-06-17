#ifndef RXDGACTIVATIONTOKEN_H
#define RXDGACTIVATIONTOKEN_H

#include <CZ/Louvre/LResource.h>
#include <string>
#include <CZ/CZWeak.h>

class Louvre::Protocols::XdgActivation::RXdgActivationToken final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void set_serial(wl_client *client, wl_resource *resource, UInt32 serial, wl_resource *seat);
    static void set_app_id(wl_client *client, wl_resource *resource, const char *app_id);
    static void set_surface(wl_client *client, wl_resource *resource, wl_resource *surface);
    static void commit(wl_client *client, wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);

    /******************** EVENTS ********************/

    void done(const std::string &token) noexcept;

private:
    friend class Louvre::Protocols::XdgActivation::GXdgActivation;
    RXdgActivationToken(GXdgActivation *xdgActivationRes,
                        UInt32 id) noexcept;
    ~RXdgActivationToken() = default;
    CZWeak<LSurface> m_surface;
    std::string m_appId;
    UInt32 m_serial { 0 };
    bool m_commited { false };
};

#endif // RXDGACTIVATIONTOKEN_H
