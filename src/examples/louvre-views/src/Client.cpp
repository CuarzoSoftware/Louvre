#include <LLog.h>
#include <unistd.h>
#include <string.h>
#include <cstdio>
#include <LSeat.h>
#include "Client.h"
#include "App.h"
#include "Global.h"
#include "Surface.h"

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

Client::Client(Params *params) : LClient(params),
    unresponsiveAnim(1000,
    [this](LAnimation *anim)
    {
        Float32 color;

        if (unresponsiveCount > 0)
            color = 1.f - 0.8 * anim->value();
        else
            color = 0.2f + 0.8 * anim->value();

        for (Surface *s : (std::list<Surface*>&)surfaces())
            s->view->setColorFactor(color, color, color, 1.f);

        compositor()->repaintAllOutputs();
    },
    [this](LAnimation *)
    {
        Float32 color = 1.f;

        if (unresponsiveCount > 0)
            color = 0.2f;

        for (Surface *s : (std::list<Surface*>&)surfaces())
        {
            s->view->setColorFactor(color, color, color, 1.f);
            s->requestNextFrame(false);
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
                    destroy();
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

    wl_client_get_credentials(client(), &pid, NULL, NULL);

    // Compositor pid
    Int32 cpid = getpid();
    Int32 ppid = pid;

    // Search the AppDock item that started it (if any)
    while (ppid != 1 && ppid != cpid)
    {
        for (App *app : G::apps())
        {
            if (app->pid == ppid)
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

void Client::pong(UInt32 serial)
{
    lastPong = serial;
}

void Client::createNonPinnedApp()
{
    if (app)
        return;

    char name[256];
    getProcessNameByPid(pid, name, sizeof(name));

    LLog::debug("[louvre-views] Non pinned app name: %s", name);

    // If not a pinned Dock app, create a tmp AppDock item
    app = new App(name, NULL, NULL);
    app->client = this;
    app->pid = pid;
}
