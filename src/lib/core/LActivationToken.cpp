#include <LActivationTokenManager.h>
#include <LClient.h>
#include <LSurface.h>

using namespace Louvre;

const std::unordered_map<std::string, LActivationToken>::iterator LActivationToken::destroy() const noexcept
{
    auto &tokens { Louvre::activationTokenManager()->m_tokens };
    return tokens.erase(tokens.find(*m_key));
}

LActivationToken::LActivationToken(LClient *creator, LSurface *origin, LEvent *triggeringEvent, std::string &&toActivateAppId) noexcept :
    m_creator(creator),
    m_origin(origin),
    m_triggeringEvent(triggeringEvent),
    m_toActivateAppId(toActivateAppId)
{}
