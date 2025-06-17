#ifndef RXDGTOPLEVELDECORATION_H
#define RXDGTOPLEVELDECORATION_H

#include <CZ/Louvre/LResource.h>
#include <CZ/CZWeak.h>

class Louvre::Protocols::XdgDecoration::RXdgToplevelDecoration final : public LResource
{
public:
    LToplevelRole *toplevelRole() const noexcept
    {
        return m_toplevelRole;
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void set_mode(wl_client *client, wl_resource *resource, UInt32 mode);
    static void unset_mode(wl_client *client, wl_resource *resource);

    /******************** EVENTS ********************/

    // Since 1
    void configure(UInt32 mode) noexcept;

private:
    friend class Louvre::Protocols::XdgDecoration::GXdgDecorationManager;
    RXdgToplevelDecoration(GXdgDecorationManager *xdgDecorationManagerRes,
                           LToplevelRole *toplevelRole,
                           UInt32 id) noexcept;

    ~RXdgToplevelDecoration();
    CZWeak<LToplevelRole> m_toplevelRole;
};

#endif // RXDGTOPLEVELDECORATION_H
