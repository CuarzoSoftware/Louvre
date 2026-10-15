#include <CZ/Louvre/Private/LFreezeWatchdog.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/CZTime.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

#include <algorithm>
#include <format>
#include <cstdlib>

#include <fcntl.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/wait.h>

using namespace CZ;

LFreezeWatchdog *LFreezeWatchdog::Get() noexcept
{
    static LFreezeWatchdog *instance { []() -> LFreezeWatchdog* {

        const char *env { getenv("CZ_LOUVRE_FREEZE_WATCHDOG_MS") };

        if (!env)
            return nullptr;

        const auto thresholdMs { atoll(env) };

        if (thresholdMs <= 0)
            return nullptr;

        // Never destroyed on purpose: outlives every output and costs nothing (debug tool)
        return new LFreezeWatchdog(static_cast<UInt64>(thresholdMs) * 1000);
    }() };

    return instance;
}

LFreezeWatchdog::LFreezeWatchdog(UInt64 thresholdUs) noexcept : m_thresholdUs(thresholdUs)
{
    // Lets the forked gdb attach even with ptrace_scope = 1
    prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);

    LLog(CZWarning, CZLN, "[watchdog] Freeze watchdog enabled (threshold {} ms)", thresholdUs / 1000);
    m_thread = std::thread([this]() { loop(); });
}

LFreezeWatchdog::~LFreezeWatchdog()
{
    m_running.store(false);
    m_thread.join();
}

void LFreezeWatchdog::watch(LOutput *output) noexcept
{
    std::lock_guard<std::mutex> lock { m_mutex };

    if (std::find(m_outputs.begin(), m_outputs.end(), output) == m_outputs.end())
        m_outputs.emplace_back(output);
}

void LFreezeWatchdog::unwatch(LOutput *output) noexcept
{
    std::lock_guard<std::mutex> lock { m_mutex };
    CZVectorUtils::RemoveOneUnordered(m_outputs, output);
}

void LFreezeWatchdog::loop() noexcept
{
    const UInt64 pollUs { std::clamp<UInt64>(m_thresholdUs / 4, 10000, 100000) };

    while (m_running.load())
    {
        usleep(pollUs);

        // Suspended session (e.g. VT switch): outputs legitimately stop presenting
        if (!seat()->enabled())
            continue;

        const UInt64 nowUs { CZTime::Us() };

        LOutput *stuckOutput {};
        UInt64 stuckUs { 0 };

        {
            std::lock_guard<std::mutex> lock { m_mutex };

            for (LOutput *output : m_outputs)
            {
                // A frame being painted right now, or pending paint since its first repaint() request
                UInt64 sinceUs { output->imp()->paintSinceUs.load() };

                if (sinceUs == 0)
                    sinceUs = output->imp()->repaintRequestUs.load();

                if (sinceUs != 0 && nowUs - sinceUs > m_thresholdUs && nowUs - sinceUs > stuckUs)
                {
                    stuckOutput = output;
                    stuckUs = nowUs - sinceUs;
                }
            }
        }

        // Cooldown so a long freeze doesn't produce a dump per poll
        if (stuckOutput && (m_lastDumpUs == 0 || nowUs - m_lastDumpUs > 5000000))
        {
            m_lastDumpUs = nowUs;
            dumpStacks(stuckOutput, stuckUs);
        }
    }
}

void LFreezeWatchdog::dumpStacks(LOutput *output, UInt64 stuckUs) noexcept
{
    const pid_t self { getpid() };
    const auto path { std::format("/tmp/cz-louvre-freeze-{}-{}.txt", self, ++m_dumpCount) };

    LLog(CZWarning, CZLN, "[watchdog] Output {} frame stuck for {} ms. Dumping thread stacks to {}",
         output->name(), stuckUs / 1000, path);

    const pid_t pid { fork() };

    if (pid == 0)
    {
        const int fd { open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644) };

        if (fd >= 0)
        {
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
        }

        char pidStr[16];
        snprintf(pidStr, sizeof(pidStr), "%d", self);

        execlp("gdb", "gdb", "-p", pidStr, "-batch",
               "-ex", "set pagination off",
               "-ex", "thread apply all bt",
               static_cast<char*>(nullptr));
        _exit(127);
    }
    else if (pid > 0)
    {
        // The freeze is already in progress; blocking this thread while gdb works is fine
        int status;
        waitpid(pid, &status, 0);
        LLog(CZWarning, CZLN, "[watchdog] Stack dump finished: {}", path);
    }
    else
        LLog(CZError, CZLN, "[watchdog] fork() failed, could not dump stacks");
}
