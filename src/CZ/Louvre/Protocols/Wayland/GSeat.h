#ifndef GSEAT_H
#define GSEAT_H

#include <LResource.h>
#include <CZ/CZWeak.h>

class Louvre::Protocols::Wayland::GSeat final : public LResource
{
public:

    const std::vector<RKeyboard*> &keyboardRes() const noexcept { return m_keyboardRes; }
    const std::vector<RPointer*> &pointerRes() const noexcept { return m_pointerRes; }
    const std::vector<RTouch *> &touchRes() const noexcept { return m_touchRes; }
    RDataDevice *dataDeviceRes() const noexcept { return m_dataDeviceRes; }

    /******************** REQUESTS ********************/

    static void get_pointer(wl_client *client, wl_resource *resource, UInt32 id) noexcept;
    static void get_keyboard(wl_client *client, wl_resource *resource, UInt32 id) noexcept;
    static void get_touch(wl_client *client, wl_resource *resource, UInt32 id) noexcept;
#if LOUVRE_WL_SEAT_VERSION >= 5
    static void release(wl_client *client, wl_resource *resource) noexcept;
#endif

    /******************** EVENTS ********************/

    // Since 1
    void capabilities(UInt32 capabilities) noexcept;

    // Since 2
    bool name(const char *name) noexcept;

private:
    friend class RPointer;
    friend class RKeyboard;
    friend class RTouch;
    friend class RDataDevice;
    LGLOBAL_INTERFACE
    GSeat(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GSeat() noexcept;
    std::vector<RPointer*> m_pointerRes;
    std::vector<RKeyboard*> m_keyboardRes;
    std::vector<RTouch*> m_touchRes;
    CZWeak<RDataDevice> m_dataDeviceRes;
};

#endif // GSEAT_H
