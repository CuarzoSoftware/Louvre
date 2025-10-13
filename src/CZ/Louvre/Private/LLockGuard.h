#ifndef CZ_LLOCKGUARD_H
#define CZ_LLOCKGUARD_H

namespace CZ
{
    struct LLockGuard
    {
        // Creates a scoped lock
        LLockGuard() noexcept : didLock(LLockGuard::Lock()) {}

        ~LLockGuard() noexcept
        {
            if (didLock)
                LLockGuard::Unlock();
        }

        // true if locked, false if already locked
        static bool Lock() noexcept;

        // true if unlocked, false if already unlocked
        static bool Unlock() noexcept;
    private:
        bool didLock;
    };
}

#endif // CZ_LLOCKGUARD_H
