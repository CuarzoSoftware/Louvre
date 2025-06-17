#ifndef RGESTUREPINCH_H
#define RGESTUREPINCH_H

#include <CZ/Louvre/LResource.h>
#include <CZ/CZWeak.h>

class Louvre::Protocols::PointerGestures::RGesturePinch final : public LResource
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
    void begin(const LPointerPinchBeginEvent &event, Wayland::RSurface *surfaceRes) noexcept;
    void update(const LPointerPinchUpdateEvent &event) noexcept;
    void end(const LPointerPinchEndEvent &event) noexcept;

private:
    friend class Louvre::Protocols::PointerGestures::GPointerGestures;
    RGesturePinch(Wayland::RPointer *pointerRes, Int32 id, UInt32 version) noexcept;
    ~RGesturePinch() noexcept;
    CZWeak<Wayland::RPointer> m_pointerRes;
};

#endif // RGESTUREPINCH_H
