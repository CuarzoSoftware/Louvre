#ifndef RTOUCH_H
#define RTOUCH_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RTouch final : public LResource
{
public:
    GSeat *seatRes() const noexcept
    {
        return m_seatRes.get();
    }

    /******************** REQUESTS ********************/

#if LOUVRE_WL_SEAT_VERSION >= 3
    static void release(wl_client *client, wl_resource *resource) noexcept;
#endif

    /******************** EVENTS ********************/

    // Since 1
    void down(const LTouchDownEvent &event, RSurface *surfaceRes) noexcept;
    void up(const LTouchUpEvent &event) noexcept;
    void motion(UInt32 time, Int32 id, Float24 x, Float24 y) noexcept;
    void frame() noexcept;
    void cancel() noexcept;

    // Since 6
    bool shape(Int32 id, Float24 major, Float24 minor) noexcept;
    bool orientation(Int32 id, Float24 orientation) noexcept;

private:
    friend class Louvre::Protocols::Wayland::GSeat;
    RTouch(GSeat *seatRes, Int32 id) noexcept;
    ~RTouch() noexcept;
    LWeak<GSeat> m_seatRes;
};

#endif // RTOUCH_H
