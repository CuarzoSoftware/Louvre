#ifndef RWLROUTPUTMODE_H
#define RWLROUTPUTMODE_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>
#include <memory>

class CZ::Protocols::WlrOutputManagement::RWlrOutputMode final : public LResource
{
public:

    std::shared_ptr<LOutputMode> mode() const noexcept
    {
        return m_mode;
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
    RWlrOutputMode(RWlrOutputHead *wlrOutputHead, std::shared_ptr<LOutputMode> mode) noexcept;
    ~RWlrOutputMode() noexcept;
    CZWeak<RWlrOutputHead> m_wlrOutputHead;
    std::shared_ptr<LOutputMode> m_mode;
};

#endif // RWLROUTPUTMODE_H
