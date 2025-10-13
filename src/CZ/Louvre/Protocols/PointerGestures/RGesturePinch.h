#ifndef RGESTUREPINCH_H
#define RGESTUREPINCH_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>

class CZ::Protocols::PointerGestures::RGesturePinch final : public LResource
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
    void begin(const CZPointerPinchBeginEvent &event, Wayland::RWlSurface *surfaceRes) noexcept;
    void update(const CZPointerPinchUpdateEvent &event) noexcept;
    void end(const CZPointerPinchEndEvent &event) noexcept;

private:
    friend class CZ::Protocols::PointerGestures::GPointerGestures;
    RGesturePinch(Wayland::RPointer *pointerRes, Int32 id, UInt32 version) noexcept;
    ~RGesturePinch() noexcept;
    CZWeak<Wayland::RPointer> m_pointerRes;
};

#endif // RGESTUREPINCH_H
