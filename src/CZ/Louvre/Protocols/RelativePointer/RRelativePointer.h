#ifndef RRELATIVEPOINTER_H
#define RRELATIVEPOINTER_H

#include <CZ/Louvre/LResource.h>
#include <CZ/CZWeak.h>

class Louvre::Protocols::RelativePointer::RRelativePointer final : public LResource
{
public:
    Wayland::RPointer *pointerRes() const noexcept
    {
        return m_pointerRes;
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
    CZWeak<Wayland::RPointer> m_pointerRes;
};

#endif // RRELATIVEPOINTER_H
