#ifndef RRELATIVEPOINTER_H
#define RRELATIVEPOINTER_H

#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::RelativePointer::RRelativePointer final : public LResource
{
public:
    Wayland::RPointer *pointerRes() const noexcept
    {
        return m_pointerRes.get();
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;

    /******************** EVENTS ********************/

    // Since 1
    void relativeMotion(const LPointerMoveEvent &event) noexcept;

private:
    friend class Louvre::Protocols::RelativePointer::GRelativePointerManager;
    RRelativePointer(Wayland::RPointer *pointerRes, Int32 id, UInt32 version) noexcept;
    ~RRelativePointer() noexcept;
    LWeak<Wayland::RPointer> m_pointerRes;
};

#endif // RRELATIVEPOINTER_H
