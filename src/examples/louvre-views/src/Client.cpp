#include <LCompositor.h>
#include <LLog.h>
#include <unistd.h>
#include <string.h>
#include <cstdio>
#include <LSeat.h>
#include "Client.h"
#include "App.h"
#include "Global.h"
#include "Surface.h"
#include "src/Compositor.h"

static int getPpidFromProc(int pid)
{
    char filename[128];
    snprintf(filename, sizeof(filename), "/proc/%d/status", pid);

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    int ppid = -1;
    char line[128];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "PPid:", 5) == 0) {
            sscanf(line + 6, "%d", &ppid);
            break;
        }
    }

    fclose(file);
    return ppid;
}

int getProcessNameByPid(int pid, char *process_name, size_t buffer_size) {
    char pid_path[1024];
    ssize_t len;

    // Construct the path to the /proc/PID/exe symbolic link
    snprintf(pid_path, sizeof(pid_path), "/proc/%d/exe", pid);

    // Read the symbolic link to get the process name
    len = readlink(pid_path, process_name, buffer_size - 1);
    if (len == -1) {
        perror("readlink");
        return -1; // Error occurred
    }

    process_name[len] = '\0'; // Null-terminate the process name

    // Extract only the actual process name (strip the path)
    char *name = strrchr(process_name, '/');
    if (name != NULL) {
        name++; // Move past the last '/'
    } else {
        name = process_name; // Use the whole name if '/' is not found
    }

    strncpy(process_name, name, buffer_size); // Copy the process name to the provided buffer
    return 0; // Success
}

Client::Client(const void *params) : LClient(params),
    unresponsiveAnim(1000,
    [this](LAnimation *anim)
    {
        Float32 color;

        if (unresponsiveCount > 0)
            color = 1.f - 0.8 * anim->value();
        else
            color = 0.2f + 0.8 * anim->value();

        for (LSurface *s : compositor()->surfaces())
            if (s->client() == this)
                static_cast<Surface*>(s)->view.setColorFactor({color, color, color, 1.f});

        compositor()->repaintAllOutputs();
    },
    [this](LAnimation *)
    {
        Float32 color = 1.f;

        if (unresponsiveCount > 0)
            color = 0.2f;

        for (LSurface *s : compositor()->surfaces())
        {
            if (s->client() == this)
            {
                static_cast<Surface*>(s)->view.setColorFactor({color, color, color, 1.f});
                static_cast<Surface*>(s)->requestNextFrame(false);
            }
        }

        compositor()->repaintAllOutputs();
    })
{

    pingTimer.setCallback([this](LTimer *)
    {
        UInt32 newPing = lastPing + 1;

        if (ping(newPing))
        {
            if (seat()->enabled() && lastPing != lastPong)
            {
                if (unresponsiveCount > 5)
                {
                    LLog::warning("[louvre-views] Destroyed unresponsive client %lu.", (UInt64)client());
                    destroyLater();
                    return;
                }
                else if (unresponsiveCount == 0)
                    unresponsiveAnim.start();

                unresponsiveCount++;
            }
            else
            {
                if (unresponsiveCount > 0)
                {
                    unresponsiveCount = 0;
                    unresponsiveAnim.start();
                }
            }
        }
        else
           lastPong = newPing;

        lastPing = newPing;
        pingTimer.start(3000);
    });

    pingTimer.start(3000);
    credentials(&pid, NULL, NULL);

    if (G::compositor()->wofi)
    {
        char name[256];
        getProcessNameByPid(pid, name, sizeof(name));

        if (strcmp(name, "wofi") == 0)
        {
            if (G::compositor()->wofi->client)
            {
                destroyLater();
                return;
            }

            app = G::compositor()->wofi;
            G::compositor()->wofi->pid = pid;
            G::compositor()->wofi->client = this;
            return;
        }
    }

    // Compositor pid
    Int32 cpid = getpid();
    Int32 ppid = pid;

    // Search the AppDock item that started it (if any)
    while (ppid != 1 && ppid != cpid)
    {
        for (App *app : G::apps())
        {
            if (app->pid == ppid && (!G::compositor()->wofi || G::compositor()->wofi->pid != ppid))
            {
                if (!app->client)
                {
                    this->app = app;
                    app->client = this;
                    return;
                }
                else
                    return;
            }
        }

        ppid = getPpidFromProc(ppid);
    }
}

Client::~Client()
{
    if (app)
    {
        // Only destroy App if is not pinned to the dock
        if (app->pinned)
        {
            app->state = App::Dead;
            app->client = nullptr;
        }
        else
            delete app;
    }
}

void Client::pong(UInt32 serial) noexcept
{
    lastPong = serial;
}

void Client::createNonPinnedApp()
{
    if (app)
        return;

    bool hasToplevel { false };

    for (LSurface *s : compositor()->surfaces())
    {
        if (s->client() == this && s->toplevel())
        {
            hasToplevel = true;
            break;
        }
    }

    if (!hasToplevel)
        return;

    char name[256];
    getProcessNameByPid(pid, name, sizeof(name));

    LLog::debug("[louvre-views] Non pinned app name: %s", name);

    // If not a pinned Dock app, create a tmp AppDock item
    app = new App(name, NULL, NULL);
    app->client = this;
    app->pid = pid;

    Int32 cpid = getpid();
    Int32 ppid = pid;

    while (ppid != 1 && ppid != cpid)
        ppid = getPpidFromProc(ppid);
}
