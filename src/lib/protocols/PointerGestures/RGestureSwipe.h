#ifndef RGESTURESWIPE_H
#define RGESTURESWIPE_H

#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::PointerGestures::RGestureSwipe final : public LResource
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
    void begin(const LPointerSwipeBeginEvent &event, Wayland::RSurface *surfaceRes) noexcept;
    void update(const LPointerSwipeUpdateEvent &event) noexcept;
    void end(const LPointerSwipeEndEvent &event) noexcept;

private:
    friend class Louvre::Protocols::PointerGestures::GPointerGestures;
    RGestureSwipe(Wayland::RPointer *pointerRes, Int32 id, UInt32 version) noexcept;
    ~RGestureSwipe() noexcept;
    LWeak<Wayland::RPointer> m_pointerRes;
};

#endif // RGESTURESWIPE_H
