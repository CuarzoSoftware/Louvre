#ifndef RLOCKEDPOINTER_H
#define RLOCKEDPOINTER_H

#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::PointerConstraints::RLockedPointer final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void set_cursor_position_hint(wl_client *client, wl_resource *resource, Float24 x, Float24 y);
    static void set_region(wl_client *client, wl_resource *resource, wl_resource *region);

    /******************** EVENTS ********************/

    // Since 1
    void locked() noexcept;
    void unlocked() noexcept;

private:
    friend class Louvre::Protocols::PointerConstraints::GPointerConstraints;
    RLockedPointer(Wayland::GOutput *outputRes, Int32 version, UInt32 id) noexcept;
    ~RLockedPointer();
    LWeak<LSurface> m_surface;
};

#endif // RLOCKEDPOINTER_H
