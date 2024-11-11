#ifndef RWLROUTPUTMODE_H
#define RWLROUTPUTMODE_H

#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::WlrOutputManagement::RWlrOutputMode final : public LResource
{
public:

    LOutputMode *mode() const noexcept
    {
        return m_mode.get();
    }

    /******************** REQUESTS ********************/

    static void release(wl_client *client, wl_resource *resource);

    /******************** EVENTS ********************/

    void size(const LSize &size) noexcept;
    void refresh(Int32 refresh) noexcept;
    void preferred() noexcept;
    void finished() noexcept;

private:
    friend class RWlrOutputHead;
    RWlrOutputMode(RWlrOutputHead *wlrOutputHead, LOutputMode *mode) noexcept;
    ~RWlrOutputMode() noexcept;
    LWeak<RWlrOutputHead> m_wlrOutputHead;
    LWeak<LOutputMode> m_mode;
};

#endif // RWLROUTPUTMODE_H
