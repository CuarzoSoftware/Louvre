#ifndef RRELATIVEPOINTER_H
#define RRELATIVEPOINTER_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>

class CZ::Protocols::RelativePointer::RRelativePointer final : public LResource
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
    void relativeMotion(const CZPointerMoveEvent &event) noexcept;

private:
    friend class CZ::Protocols::RelativePointer::GRelativePointerManager;
    RRelativePointer(Wayland::RPointer *pointerRes, Int32 id, UInt32 version) noexcept;
    ~RRelativePointer() noexcept;
    CZWeak<Wayland::RPointer> m_pointerRes;
};

#endif // RRELATIVEPOINTER_H
