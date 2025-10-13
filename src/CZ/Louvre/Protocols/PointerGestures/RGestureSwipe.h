#ifndef RGESTURESWIPE_H
#define RGESTURESWIPE_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>

class CZ::Protocols::PointerGestures::RGestureSwipe final : public LResource
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
    void begin(const CZPointerSwipeBeginEvent &event, Wayland::RWlSurface *surfaceRes) noexcept;
    void update(const CZPointerSwipeUpdateEvent &event) noexcept;
    void end(const CZPointerSwipeEndEvent &event) noexcept;

private:
    friend class CZ::Protocols::PointerGestures::GPointerGestures;
    RGestureSwipe(Wayland::RPointer *pointerRes, Int32 id, UInt32 version) noexcept;
    ~RGestureSwipe() noexcept;
    CZWeak<Wayland::RPointer> m_pointerRes;
};

#endif // RGESTURESWIPE_H
