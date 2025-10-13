#ifndef RWLROUTPUTHEAD_H
#define RWLROUTPUTHEAD_H

#include <CZ/Core/CZTransform.h>
#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>
#include <memory>

class CZ::Protocols::WlrOutputManagement::RWlrOutputHead final : public LResource
{
public:

    LOutput *output() const noexcept
    {
        return m_output.get();
    }

    const std::vector<RWlrOutputMode*> &modes() const noexcept
    {
        return m_modes;
    }

    void markAsPendingDone() noexcept;

    /******************** REQUESTS ********************/

#if LOUVRE_WLR_OUTPUT_MANAGER_VERSION >= 3
    static void release(wl_client *client, wl_resource *resource);
#endif

    /******************** EVENTS ********************/

    void name(const char *name) noexcept;
    void description(const char *description) noexcept;
    void physicalSize(const SkISize &size) noexcept;
    RWlrOutputMode *mode(std::shared_ptr<LOutputMode> mode) noexcept;
    void enabled(bool enabled) noexcept;
    void currentMode(RWlrOutputMode *mode) noexcept;
    void position(SkIPoint pos) noexcept;
    void transform(CZTransform transform) noexcept;
    void scale(Float32 scale) noexcept;
    void finished() noexcept;

    // Since 2
    bool make(const char *make) noexcept;
    bool model(const char *model) noexcept;
    bool serialNumber(const char *serial) noexcept;

    // Since 4
    bool adaptiveSync(bool enabled) noexcept;
private:
    friend class GWlrOutputManager;
    friend class RWlrOutputMode;
    RWlrOutputHead(GWlrOutputManager *wlrOutputManager, LOutput *output) noexcept;
    ~RWlrOutputHead() noexcept;
    CZWeak<GWlrOutputManager> m_wlrOutputManager;
    CZWeak<LOutput> m_output;
    std::vector<RWlrOutputMode*> m_modes;
};

#endif // RWLROUTPUTHEAD_H
