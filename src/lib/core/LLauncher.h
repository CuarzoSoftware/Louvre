#ifndef LLAUNCHER_H
#define LLAUNCHER_H

#include <LNamespaces.h>
#include <string>

/**
 * @brief Utility for launching applications safely.
 *
 * Creating a fork of the compositor while it's running is a risky operation that can result in resource leaks to child processes,
 * leading to undesired behaviors and potentially causing the compositor to experience reduced performance or crashes.
 *
 * The LLauncher class is an auxiliary class designed to facilitate the secure launching of applications from the compositor.
 * It creates a background daemon capable of launching applications using the [system()](https://man7.org/linux/man-pages/man3/system.3.html) call.
 *
 * The daemon must be started before creating an instance of LCompositor, achieved through the startDaemon() function.
 * The daemon can be terminated by calling the stopDaemon() function and is automatically exited when the compositor ends.
 *
 * If the daemon exits normally, it sends a [SIGTERM](https://www.gnu.org/software/libc/manual/html_node/Termination-Signals.html#index-SIGTERM) signal to all processes in its process group.
 */
class Louvre::LLauncher
{
public:
    /**
     * @brief Starts the daemon and returns its process ID.
     *
     * Should preferably be the first function called from the `main()` function and
     * must be called before creating an instance of LCompositor.
     *
     * @param name The process name to be set for the daemon.
     *
     * @return The process ID of the daemon if successful, or < 0 on error.
     *         Possible errors include the daemon already running or calling
     *         this function after creating an instance of LCompositor.
     */
    static pid_t startDaemon(const std::string &name = "LLauncher");

    /**
     * @brief Get the process ID (PID) of the daemon.
     *
     * @return The PID of the daemon if it is running, or a negative number if the daemon is not running.
     */
    static pid_t pid();

    /**
     * @brief Launches an application.
     *
     * This function uses the same arguments as the [system()](https://man7.org/linux/man-pages/man3/system.3.html) call.
     * It launches an application specified by the provided command and returns the application's process ID.
     *
     * @param command The command to execute, as a string.
     * @return The process ID of the launched application if successful, or a negative number on error.
     */
    static pid_t launch(const std::string &command);

    /**
     * @brief Terminates the daemon.
     *
     * Calling this method when the daemon is not running is a no-op.
     *
     * @note If the daemon is stopped while the compositor is running, it won't be able to be launched again.
     */
    static void stopDaemon();
};

#endif // LLAUNCHER_H
