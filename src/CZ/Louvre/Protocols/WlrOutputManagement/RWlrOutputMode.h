#ifndef RWLROUTPUTMODE_H
#define RWLROUTPUTMODE_H

#include <LResource.h>
#include <CZ/CZWeak.h>

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

    void size(const SkISize &size) noexcept;
    void refresh(Int32 refresh) noexcept;
    void preferred() noexcept;
    void finished() noexcept;

private:
    friend class RWlrOutputHead;
    RWlrOutputMode(RWlrOutputHead *wlrOutputHead, LOutputMode *mode) noexcept;
    ~RWlrOutputMode() noexcept;
    CZWeak<RWlrOutputHead> m_wlrOutputHead;
    CZWeak<LOutputMode> m_mode;
};

#endif // RWLROUTPUTMODE_H
