#include <CZ/Louvre/Protocols/XdgActivation/RXdgActivationToken.h>
#include <CZ/Louvre/Protocols/XdgActivation/xdg-activation-v1.h>
#include <CZ/Louvre/Protocols/XdgActivation/GXdgActivation.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Manager/LActivationTokenManager.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Core/CZTime.h>

using namespace CZ;
using namespace CZ::Protocols::XdgActivation;

static const struct xdg_activation_token_v1_interface imp
{
    .set_serial = &RXdgActivationToken::set_serial,
    .set_app_id = &RXdgActivationToken::set_app_id,
    .set_surface = &RXdgActivationToken::set_surface,
    .commit = &RXdgActivationToken::commit,
    .destroy = &RXdgActivationToken::destroy
};

RXdgActivationToken::RXdgActivationToken(
    GXdgActivation *xdgActivationRes,
    UInt32 id
    ) noexcept
    :LResource
    (
        xdgActivationRes->client(),
        &xdg_activation_token_v1_interface,
        xdgActivationRes->version(),
        id,
        &imp
    )
{}

/******************** REQUESTS ********************/

void RXdgActivationToken::set_serial(wl_client */*client*/, wl_resource *resource, UInt32 serial, wl_resource */*seat*/)
{
    auto &res { *static_cast<RXdgActivationToken*>(wl_resource_get_user_data(resource)) };

    if (res.m_commited)
    {
        res.postError(XDG_ACTIVATION_TOKEN_V1_ERROR_ALREADY_USED, "Duplicated commit.");
        return;
    }

    res.m_serial = serial;
}

void RXdgActivationToken::set_app_id(wl_client */*client*/, wl_resource *resource, const char *app_id)
{
    auto &res { *static_cast<RXdgActivationToken*>(wl_resource_get_user_data(resource)) };

    if (res.m_commited)
    {
        res.postError(XDG_ACTIVATION_TOKEN_V1_ERROR_ALREADY_USED, "Duplicated commit.");
        return;
    }

    res.m_appId = app_id;
}

void RXdgActivationToken::set_surface(wl_client */*client*/, wl_resource *resource, wl_resource *surface)
{
    auto &res { *static_cast<RXdgActivationToken*>(wl_resource_get_user_data(resource)) };

    if (res.m_commited)
    {
        res.postError(XDG_ACTIVATION_TOKEN_V1_ERROR_ALREADY_USED, "Duplicated commit.");
        return;
    }

    auto &surfaceRes { *static_cast<Protocols::Wayland::RWlSurface*>(wl_resource_get_user_data(surface)) };
    res.m_surface.reset(surfaceRes.surface());
}

static std::string randomToken() noexcept
{
    static bool seeded { false };
    static UInt32 n { 0 };
    n++;

    if (!seeded)
    {
        srand(CZTime::Ms());
        seeded = true;
    }

    static std::string dic { "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789" };
    static char random[17];

    retry:

    for (int i = 0; i < 16; i++)
            random[i] = dic[rand() % dic.length()];

    random[16] = '\0';

    std::string token = std::to_string(n) + "_" + random;

    if (activationTokenManager()->tokens().contains(token))
        goto retry;

    return token;
}

void RXdgActivationToken::commit(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RXdgActivationToken*>(wl_resource_get_user_data(resource)) };

    if (res.m_commited)
    {
        res.postError(XDG_ACTIVATION_TOKEN_V1_ERROR_ALREADY_USED, "Duplicated commit.");
        return;
    }

    res.m_commited = true;

    const CZEvent *event { res.client()->findEventBySerial(res.m_serial) };
    auto pair = activationTokenManager()->m_tokens.emplace(
        randomToken(),
        new LActivationToken(
            res.client(),
            res.m_surface,
            event ? event->copy() : nullptr,
            std::move(res.m_appId))).first;

    pair->second->m_key = &pair->first;
    activationTokenManager()->m_token.reset(pair->second);
    activationTokenManager()->createTokenRequest();

    // Request accepted
    if (activationTokenManager()->token())
    {
        activationTokenManager()->m_token.reset();
        res.done(pair->first);
    }
    // The user denied the request (destroyed the token)
    else
        res.done("INVALID_TOKEN");
}

void RXdgActivationToken::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RXdgActivationToken::done(const std::string &token) noexcept
{
    xdg_activation_token_v1_send_done(resource(), token.c_str());
}

