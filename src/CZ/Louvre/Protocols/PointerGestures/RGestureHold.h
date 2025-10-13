#ifndef RGESTUREHOLD_H
#define RGESTUREHOLD_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>

class CZ::Protocols::PointerGestures::RGestureHold final : public LResource
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
    void begin(const CZPointerHoldBeginEvent &event, Wayland::RWlSurface *surfaceRes) noexcept;
    void end(const CZPointerHoldEndEvent &event) noexcept;

private:
    friend class CZ::Protocols::PointerGestures::GPointerGestures;
    RGestureHold(Wayland::RPointer *pointerRes, Int32 id, UInt32 version) noexcept;
    ~RGestureHold() noexcept;
    CZWeak<Wayland::RPointer> m_pointerRes;
};

#endif // RGESTUREHOLD_H
