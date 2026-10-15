#ifndef CZ_LFREEZEWATCHDOG_H
#define CZ_LFREEZEWATCHDOG_H

#include <CZ/Core/Cuarzo.h>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

namespace CZ
{
    class LOutput;

    /**
     * @brief Debugging aid for diagnosing compositor freezes.
     *
     * Disabled unless the CZ_LOUVRE_FREEZE_WATCHDOG_MS env is set to a threshold in ms (> 0).
     *
     * A monitoring thread checks every watched output's pending frame: if a repaint request
     * hasn't been painted, or a paintGL is still running, after the threshold, it attaches
     * gdb to the process and dumps every thread's stack trace to
     * /tmp/cz-louvre-freeze-<pid>-<n>.txt, capturing exactly where the freeze is happening.
     *
     * Checks are skipped while the seat is disabled (session suspended, e.g. after a VT switch).
     */
    class LFreezeWatchdog
    {
    public:
        /// nullptr unless CZ_LOUVRE_FREEZE_WATCHDOG_MS is set
        static LFreezeWatchdog *Get() noexcept;

        void watch(LOutput *output) noexcept;
        void unwatch(LOutput *output) noexcept;

    private:
        LFreezeWatchdog(UInt64 thresholdUs) noexcept;
        ~LFreezeWatchdog();

        void loop() noexcept;
        void dumpStacks(LOutput *output, UInt64 stuckUs) noexcept;

        std::mutex m_mutex;
        std::vector<LOutput*> m_outputs;
        std::thread m_thread;
        std::atomic<bool> m_running { true };
        UInt64 m_thresholdUs;
        UInt64 m_lastDumpUs { 0 };
        UInt32 m_dumpCount { 0 };
    };
}

#endif // CZ_LFREEZEWATCHDOG_H
