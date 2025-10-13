#ifndef CZ_LRESOURCEREF_H
#define CZ_LRESOURCEREF_H

#include <CZ/Core/CZTime.h>
#include <wayland-server.h>
#include <list>

namespace CZ
{
    // Weak wl_resource ref
    class LResourceRef
    {
    public:
        LResourceRef(wl_resource *res = nullptr) noexcept;
        ~LResourceRef() noexcept;

        LResourceRef(const LResourceRef& other) noexcept;
        LResourceRef(LResourceRef&& other) noexcept;

        LResourceRef& operator=(const LResourceRef& other) noexcept;
        LResourceRef& operator=(LResourceRef&& other) noexcept;

        void setResource(wl_resource *resource) noexcept;
        wl_resource *res() const noexcept { return m_res; }
        static void HandleDestroy(wl_listener *listener, void *data) noexcept;

    private:
        wl_listener m_onDestroy;
        wl_resource *m_res{};
    };

    // Manage wl_surface.frames
    struct LFrameCallbacks
    {
        ~LFrameCallbacks() noexcept { sendDoneAndDestroyFrames(); }

        // Send done and destroy frames in post order
        void sendDoneAndDestroyFrames() noexcept;
        std::list<LResourceRef> resources;
    };
}

#endif
