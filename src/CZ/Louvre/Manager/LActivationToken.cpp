#include <CZ/Louvre/Manager/LActivationTokenManager.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/LLog.h>

using namespace CZ;

const std::unordered_map<std::string, LActivationToken*>::iterator LActivationToken::destroy() const noexcept
{
    auto pos { CZ::activationTokenManager()->m_tokens.find(*m_key) };
    auto it { CZ::activationTokenManager()->m_tokens.erase(pos) };
    delete const_cast<LActivationToken*>(this);
    return it;
}

LActivationToken::LActivationToken(LClient *creator, LSurface *origin, std::shared_ptr<CZEvent> triggeringEvent, std::string &&toActivateAppId) noexcept :
    m_creator(creator),
    m_origin(origin),
    m_triggeringEvent(triggeringEvent),
    m_toActivateAppId(toActivateAppId)
{}
