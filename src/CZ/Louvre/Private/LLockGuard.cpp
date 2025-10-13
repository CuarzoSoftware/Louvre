#include <CZ/Louvre/Private/LLockGuard.h>
#include <mutex>

using namespace CZ;

static std::mutex Mutex;
thread_local bool DidThreadLock { false };

bool LLockGuard::Lock() noexcept
{
    if (DidThreadLock)
        return false;

    DidThreadLock = true;
    Mutex.lock();
    return true;
}

bool LLockGuard::Unlock() noexcept
{
    if (!DidThreadLock)
        return false;

    DidThreadLock = false;
    Mutex.unlock();    
    return true;
}
