#ifndef GFOREIGNTOPLEVELLIST_H
#define GFOREIGNTOPLEVELLIST_H

#include <LResource.h>

class Louvre::Protocols::ForeignToplevelList::GForeignToplevelList final : public LResource
{
public:

    /*
     * Checks if stop() has been requested.
     *
     * If true:
     * - Further client requests will be ignored.
     * - New toplevel objects won't be announced.
     * - Changes from already announced toplevels will still be notified.
     */
    bool stopRequested() const noexcept
    {
        return m_stopped;
    }

    // No more toplevel events are sent
    bool finishedSent() const noexcept
    {
        return m_finished;
    }

    /******************** REQUESTS ********************/

    static void stop(wl_client *client, wl_resource *resource) noexcept;
    static void destroy(wl_client *client, wl_resource *resource) noexcept;

    /******************** EVENTS ********************/

    /* Creates a new handle */
    void toplevel(LToplevelRole &toplevelRole);

    /* Notify and destroy the resource */
    const std::vector<GForeignToplevelList*>::iterator finished();

private:
    friend class Louvre::Protocols::ForeignToplevelList::RForeignToplevelHandle;
    bool m_stopped { false };
    bool m_finished { false };
    LGLOBAL_INTERFACE
    GForeignToplevelList(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GForeignToplevelList() noexcept;
};

#endif // GFOREIGNTOPLEVELLIST_H
