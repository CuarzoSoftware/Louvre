#include <CZ/Louvre/Protocols/GammaControl/wlr-gamma-control-unstable-v1.h>
#include <CZ/Louvre/Protocols/GammaControl/GZwlrGammaControlManagerV1.h>
#include <CZ/Louvre/Protocols/GammaControl/RZwlrGammaControlV1.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>
#include <CZ/Louvre/Backends/LBackend.h>

using namespace CZ::Protocols::GammaControl;

static const struct zwlr_gamma_control_manager_v1_interface imp
{
    .get_gamma_control = &GZwlrGammaControlManagerV1::get_gamma_control,
    .destroy = &GZwlrGammaControlManagerV1::destroy
};

LGLOBAL_INTERFACE_IMP(GZwlrGammaControlManagerV1, LOUVRE_GAMMA_CONTROL_MANAGER_VERSION, zwlr_gamma_control_manager_v1_interface)

bool GZwlrGammaControlManagerV1::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->backend()->id() != LBackendId::DRM)
    {
        LLog(CZWarning, CZLN, "Failed to create {} global (unsupported by the current backend)", Interface()->name);
        return false;
    }

    if (compositor()->wellKnownGlobals.WlrGammaControlManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.WlrGammaControlManager;
    return true;
}

GZwlrGammaControlManagerV1::GZwlrGammaControlManagerV1(
    wl_client *client,
    Int32 version,
    UInt32 id
)
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->gammaControlManagerGlobals.emplace_back(this);
}

GZwlrGammaControlManagerV1::~GZwlrGammaControlManagerV1() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->gammaControlManagerGlobals, this);
}

void GZwlrGammaControlManagerV1::get_gamma_control(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *output) noexcept
{
    new RZwlrGammaControlV1(static_cast<Wayland::GOutput*>(wl_resource_get_user_data(output)),
                      wl_resource_get_version(resource),
                      id);
}

/******************** REQUESTS ********************/

void GZwlrGammaControlManagerV1::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
