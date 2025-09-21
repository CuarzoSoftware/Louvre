#include "Client.h"

#include <LCompositor.h>
#include <LLog.h>
#include <LSeat.h>
#include <string.h>
#include <unistd.h>

#include <cstdio>

#include "App.h"
#include "Global.h"
#include "Surface.h"
#include "src/Compositor.h"

static Int32 getPpidFromProc(Int32 pid) {
  char filename[128];
  snprintf(filename, sizeof(filename), "/proc/%d/status", pid);

  FILE *file{fopen(filename, "r")};

  if (file == NULL) {
    LLog::error("[louvre-views][Client::getPpidFromProc] Error opening %s.",
                filename);
    return -1;
  }

  Int32 ppid{-1};
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

static Int32 getProcessNameByPid(Int32 pid, char *processName,
                                 size_t bufferSize) {
  char pid_path[1024];
  ssize_t len;

  snprintf(pid_path, sizeof(pid_path), "/proc/%d/exe", pid);

  // Read the symbolic link to get the process name
  len = readlink(pid_path, processName, bufferSize - 1);

  if (len == -1) {
    LLog::error("[louvre-views][Client::getProcessNameByPid] readlink failed.");
    return -1;
  }

  processName[len] = '\0';

  // Strip path
  char *name = strrchr(processName, '/');

  if (name != NULL)
    name++;
  else
    name = processName;

  strncpy(processName, name, bufferSize);
  return 0;
}

Client::Client(const void *params)
    : LClient(params),
      unresponsiveAnim(
          1000,
          [this](LAnimation *anim) {
            Float32 color;

            if (unresponsiveCount > 0)
              color = 1.f - 0.8 * anim->value();
            else
              color = 0.2f + 0.8 * anim->value();

            for (LSurface *s : compositor()->surfaces())
              if (s->client() == this)
                static_cast<Surface *>(s)->view.setColorFactor(
                    {color, color, color, 1.f});

            compositor()->repaintAllOutputs();
          },
          [this](LAnimation *) {
            Float32 color = 1.f;

            if (unresponsiveCount > 0) color = 0.2f;

            for (LSurface *s : compositor()->surfaces()) {
              if (s->client() == this) {
                static_cast<Surface *>(s)->view.setColorFactor(
                    {color, color, color, 1.f});
                static_cast<Surface *>(s)->requestNextFrame(false);
              }
            }

            compositor()->repaintAllOutputs();
          }) {
  pingTimer.setCallback([this](LTimer *) {
    const UInt32 newPingSerial{lastPingSerial + 1};

    if (ping(newPingSerial)) {
      if (seat()->enabled() && lastPingSerial != lastPongSerial) {
        if (unresponsiveCount > 5) {
          LLog::warning("[louvre-views] Destroyed unresponsive client PID(%d).",
                        pid);
          destroyLater();
          return;
        } else if (unresponsiveCount == 0)
          unresponsiveAnim.start();

        unresponsiveCount++;
      } else {
        if (unresponsiveCount > 0) {
          unresponsiveCount = 0;
          unresponsiveAnim.start();
        }
      }
    } else
      lastPongSerial = newPingSerial;

    lastPingSerial = newPingSerial;
    pingTimer.start(3000);
  });

  pingTimer.start(3000);
  credentials(&pid, NULL, NULL);

  if (G::compositor()->wofi) {
    char name[256];
    getProcessNameByPid(pid, name, sizeof(name));

    if (strcmp(name, "wofi") == 0) {
      if (G::compositor()->wofi->client) {
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
  while (ppid > cpid) {
    for (App *app : G::apps()) {
      if (app->pid == ppid &&
          (!G::compositor()->wofi || G::compositor()->wofi->pid != ppid)) {
        if (!app->client) {
          this->app = app;
          app->client = this;
          return;
        } else
          return;
      }
    }

    ppid = getPpidFromProc(ppid);
  }
}

Client::~Client() {
  if (app) {
    // Only destroy App if is not pinned to the dock
    if (app->pinned) {
      app->state = App::Dead;
      app->client = nullptr;
    } else
      delete app;
  }
}

void Client::pong(UInt32 serial) noexcept { lastPongSerial = serial; }

void Client::createUnpinnedApp() {
  if (app) return;

  bool hasToplevel{false};

  for (LSurface *s : compositor()->surfaces()) {
    if (s->client() == this && s->toplevel()) {
      hasToplevel = true;
      break;
    }
  }

  if (!hasToplevel) return;

  char name[256];
  getProcessNameByPid(pid, name, sizeof(name));

  LLog::debug("[louvre-views] Non pinned app name: %s", name);

  // If not a pinned Dock app, create a tmp AppDock item
  app = new App(name, NULL, NULL);
  app->client = this;
  app->pid = pid;

  Int32 cpid = getpid();
  Int32 ppid = pid;

  while (ppid > cpid) ppid = getPpidFromProc(ppid);
}
