#ifndef RWLROUTPUTCONFIGURATIONHEAD_H
#define RWLROUTPUTCONFIGURATIONHEAD_H

#include <CZ/CZTransform.h>
#include <LResource.h>
#include <CZ/CZWeak.h>
#include <CZ/skia/core/SkPoint.h>
#include <CZ/CZBitset.h>

class Louvre::Protocols::WlrOutputManagement::RWlrOutputConfigurationHead final : public LResource
{
public:

    enum SetProperties : UInt32
    {
        Mode            = 1 << 0,
        CustomMode      = 1 << 1,
        Position        = 1 << 2,
        Transform       = 1 << 3,
        Scale           = 1 << 4,
        VRR             = 1 << 5
    };

    LOutput *output() const noexcept
    {
        return m_output.get();
    }

    /******************** REQUESTS ********************/

    static void set_mode(wl_client *client, wl_resource *resource, wl_resource *mode);
    static void set_custom_mode(wl_client *client, wl_resource *resource, Int32 width, Int32 height, Int32 refresh);
    static void set_position(wl_client *client, wl_resource *resource, Int32 x, Int32 y);
    static void set_scale(wl_client *client, wl_resource *resource, Float24 scale);
    static void set_transform(wl_client *client, wl_resource *resource, Int32 transform);
#if LOUVRE_WLR_OUTPUT_MANAGER_VERSION >= 4
    static void set_adaptive_sync(wl_client *client, wl_resource *resource, UInt32 vrr);
#endif

private:
    friend class RWlrOutputConfiguration;
    RWlrOutputConfigurationHead(RWlrOutputConfiguration *wlrOutputConfiguration, UInt32 id, LOutput *output) noexcept;
    ~RWlrOutputConfigurationHead() noexcept;
    CZBitset<SetProperties> m_setProps;
    CZWeak<RWlrOutputConfiguration> m_wlrOutputConfiguration;
    CZWeak<LOutput> m_output;
    CZWeak<const LOutputMode> m_mode;
    SkIPoint m_pos;
    CZTransform m_transform;
    Float32 m_scale;
    bool m_vrr;
};

#endif // RWLROUTPUTCONFIGURATIONHEAD_H
