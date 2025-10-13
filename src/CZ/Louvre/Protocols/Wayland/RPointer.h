#ifndef RPOINTER_H
#define RPOINTER_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Louvre/Seat/LPointer.h>
#include <CZ/Core/CZWeak.h>

class CZ::Protocols::Wayland::RPointer final : public LResource
{
public:

    GSeat *seatRes() const noexcept
    {
        return m_seatRes;
    }

    const std::vector<Protocols::RelativePointer::RRelativePointer*> &relativePointerRes() const noexcept
    {
        return m_relativePointerRes;
    }

    const std::vector<Protocols::PointerGestures::RGestureSwipe*> &gestureSwipeRes() const noexcept
    {
        return m_gestureSwipeRes;
    }

    const std::vector<Protocols::PointerGestures::RGesturePinch*> &gesturePinchRes() const noexcept
    {
        return m_gesturePinchRes;
    }

    const std::vector<Protocols::PointerGestures::RGestureHold*> &gestureHoldRes() const noexcept
    {
        return m_gestureHoldRes;
    }

    /******************** REQUESTS ********************/

    static void set_cursor(wl_client *client, wl_resource *resource, UInt32 serial, wl_resource *surface, Int32 hotspot_x, Int32 hotspot_y);
#if LOUVRE_WL_SEAT_VERSION >= 5
    static void release(wl_client *client, wl_resource *resource) noexcept;
#endif

    /******************** EVENTS ********************/

    // Since 1
    void enter(const CZPointerEnterEvent &event, RWlSurface *rSurface) noexcept;
    void leave(const CZPointerLeaveEvent &event, RWlSurface *rSurface) noexcept;
    void motion(const CZPointerMoveEvent &event) noexcept;
    void button(const CZPointerButtonEvent &event) noexcept;
    void axis(UInt32 time, UInt32 axis, Float32 value) noexcept;

    // Since 5
    bool frame() noexcept;
    bool axisSource(UInt32 axisSource) noexcept;
    bool axisStop(UInt32 time, UInt32 axis) noexcept;
    bool axisDiscrete(UInt32 axis, Int32 discrete) noexcept;

    // Since 8
    bool axisValue120(UInt32 axis, Int32 value120) noexcept;

    // Since 9
    bool axisRelativeDirection(UInt32 axis, UInt32 direction) noexcept;

private:
    friend class CZ::Protocols::Wayland::GSeat;
    friend class CZ::Protocols::RelativePointer::RRelativePointer;
    friend class CZ::Protocols::PointerGestures::RGestureSwipe;
    friend class CZ::Protocols::PointerGestures::RGesturePinch;
    friend class CZ::Protocols::PointerGestures::RGestureHold;

    RPointer(GSeat *seatRes, Int32 id) noexcept;
    ~RPointer() noexcept;

    std::vector<Protocols::RelativePointer::RRelativePointer*> m_relativePointerRes;
    std::vector<Protocols::PointerGestures::RGestureSwipe*> m_gestureSwipeRes;
    std::vector<Protocols::PointerGestures::RGesturePinch*> m_gesturePinchRes;
    std::vector<Protocols::PointerGestures::RGestureHold*> m_gestureHoldRes;
    CZWeak<GSeat> m_seatRes;
};
#endif // RPOINTER_H
