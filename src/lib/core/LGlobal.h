#ifndef LGLOBAL_H
#define LGLOBAL_H

#include <LObject.h>

/**
 * @brief A Wayland protocol global.
 *
 * The LGlobal class encapsulates a [wl_global](https://wayland.freedesktop.org/docs/html/apc.html#Server-structwl__global),
 * which is a special protocol interface notified to clients through the [wl_registry](https://wayland.app/protocols/wayland#wl_registry) interface.
 *
 * Clients bound to a global can request the creation of resources with interfaces that provide functionality specific to the protocol
 * they belong to.
 *
 * Every protocol typically includes one or more globals. To view the globals implemented by Louvre, refer to LCompositor::createGlobalsRequest().
 *
 * You can override LCompositor::createGlobalsRequest() to disable or add custom globals. To add a custom global, refer to LCompositor::createGlobal()
 * and to remove it, use LCompositor::removeGlobal().
 *
 * @note Calling LCompositor::removeGlobal() isn't mandatory as globals are automatically destroyed during the compositor's uninitialization.
 */
class Louvre::LGlobal final : public LObject
{
public:

    /**
     * @brief Get the user data provided through LCompositor::createGlobal().
     */
    void *userData() const noexcept;

    /**
     * @brief Get the interface of the global.
     */
    const wl_interface *interface() const noexcept;

    /**
     * @brief Get the native global handle.
     */
    const wl_global *global() const noexcept
    {
        return m_global;
    }

    /// @cond OMIT
private:
    friend class LCompositor;
    LGlobal(wl_global *global) noexcept;
    ~LGlobal() noexcept;
    wl_global *m_global;
    bool m_removed { false };
    UInt8 m_destroyRoundtrips { 0 };
    /// @endcond
};

#endif // LGLOBAL_H
