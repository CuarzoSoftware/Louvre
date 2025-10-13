#include <CZ/Louvre/Protocols/XdgActivation/RXdgActivationToken.h>
#include <CZ/Louvre/Protocols/XdgActivation/xdg-activation-v1.h>
#include <CZ/Louvre/Protocols/XdgActivation/GXdgActivation.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Manager/LActivationTokenManager.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ;
using namespace CZ::Protocols::XdgActivation;

static const struct xdg_activation_v1_interface imp
{
    .destroy = &GXdgActivation::destroy,
    .get_activation_token = &GXdgActivation::get_activation_token,
    .activate = &GXdgActivation::activate,
};

LGLOBAL_INTERFACE_IMP(GXdgActivation, LOUVRE_XDG_ACTIVATION_VERSION, xdg_activation_v1_interface)

bool GXdgActivation::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.XdgActivation)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.XdgActivation;
    return true;
}

GXdgActivation::GXdgActivation(
    wl_client *client,
    Int32 version,
    UInt32 id)
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->xdgActivationGlobals.emplace_back(this);
}

GXdgActivation::~GXdgActivation() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->xdgActivationGlobals, this);
}

void GXdgActivation::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GXdgActivation::get_activation_token(wl_client */*client*/, wl_resource *resource, UInt32 id) noexcept
{
    auto *res { static_cast<GXdgActivation*>(wl_resource_get_user_data(resource)) };
    new RXdgActivationToken(res, id);
}

void GXdgActivation::activate(wl_client */*client*/, wl_resource */*resource*/, const char *token, wl_resource *surface)
{
    const std::string tokenString { token };
    auto it { activationTokenManager()->m_tokens.find(tokenString) };

    if (it == activationTokenManager()->m_tokens.end())
        return;

    auto *surfaceRes { static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface)) };
    activationTokenManager()->m_token.reset(it->second);
    activationTokenManager()->activateSurfaceRequest(surfaceRes->surface());
    activationTokenManager()->m_token.reset();
}
