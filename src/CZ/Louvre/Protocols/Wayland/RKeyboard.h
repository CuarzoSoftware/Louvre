#ifndef RKEYBOARD_H
#define RKEYBOARD_H

#include <CZ/Core/CZKeyModifiers.h>
#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>

class CZ::Protocols::Wayland::RKeyboard final : public LResource
{
public:

    GSeat *seatRes() const noexcept
    {
        return m_seatRes;
    }

    const CZKeyModifiers &lastSentModifiers() const noexcept { return m_modifiers; }

    /******************** REQUESTS ********************/

#if LOUVRE_WL_SEAT_VERSION >= 3
    static void release(wl_client *client, wl_resource *resource) noexcept;
#endif

    /******************** EVENTS ********************/

    // Since 1
    void keymap(UInt32 format, Int32 fd, UInt32 size) noexcept;
    void enter(const CZKeyboardEnterEvent &event, RWlSurface *surfaceRes, wl_array *keys) noexcept;
    void leave(const CZKeyboardLeaveEvent &event, RWlSurface *surfaceRes) noexcept;
    void key(const CZKeyboardKeyEvent &event) noexcept;
    void modifiers(const CZKeyboardModifiersEvent &event) noexcept;

    // Since 4
    bool repeatInfo(Int32 rate, Int32 delay) noexcept;

private:
    friend class CZ::Protocols::Wayland::GSeat;
    RKeyboard(GSeat *seatRes, Int32 id) noexcept;
    ~RKeyboard() noexcept;
    CZWeak<GSeat> m_seatRes;
    CZKeyModifiers m_modifiers {};
};

#endif // RKEYBOARD_H
