#include <LActivationTokenManager.h>
#include <cassert>

using namespace Louvre;

LActivationTokenManager::LActivationTokenManager(const void *params) noexcept : LFactoryObject(FactoryObjectType)
{
    assert(params != nullptr && "Invalid parameter passed to LActivationTokenManager constructor.");
    LActivationTokenManager**ptr { (LActivationTokenManager**) params };
    assert(*ptr == nullptr && *ptr == Louvre::activationTokenManager() && "Only a single LActivationTokenManager instance can exist.");
    *ptr = this;
}

LActivationTokenManager::~LActivationTokenManager() noexcept
{
    notifyDestruction();

    for (auto it = m_tokens.begin(); it != m_tokens.end();)
        it = (*it).second->destroy();
}

void LActivationTokenManager::destroyTokensOlderThanMs(UInt32 ms)
{
    if (ms == 0)
    {
        m_tokens.clear();
        return;
    }

    for (auto it = m_tokens.begin(); it != m_tokens.end();)
    {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - (*it).second->created()).count() > ms)
            it = (*it).second->destroy();
        else
            it++;
    }
}
