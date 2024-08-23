#include <LActivationTokenManager.h>
#include <LClient.h>
#include <LSurface.h>
#include <LLog.h>

using namespace Louvre;

const std::unordered_map<std::string, LActivationToken*>::iterator LActivationToken::destroy() const noexcept
{
    auto pos { Louvre::activationTokenManager()->m_tokens.find(*m_key) };
    auto it { Louvre::activationTokenManager()->m_tokens.erase(pos) };
    delete const_cast<LActivationToken*>(this);
    return it;
}

LActivationToken::LActivationToken(LClient *creator, LSurface *origin, LEvent *triggeringEvent, std::string &&toActivateAppId) noexcept :
    m_creator(creator),
    m_origin(origin),
    m_triggeringEvent(triggeringEvent),
    m_toActivateAppId(toActivateAppId)
{}
