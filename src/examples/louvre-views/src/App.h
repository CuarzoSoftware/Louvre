#ifndef APP_H
#define APP_H

#include <LObject.h>
#include "DockApp.h"

class Client;

using namespace Louvre;

class App : public LObject
{
public:

    enum AppState
    {
        Dead,
        Launching,
        Running
    };

    App(const char *name, const char *exec, const char *iconPath);
    ~App();

    void clicked();

    Client *client = nullptr;

    Int32 pid = -1;
    char name[1024];
    char exec[1024];

    // True for Apps pinned in the Dock, false for non Dock apps
    bool pinned = true;

    // App has not yet started
    AppState state = Dead;

    // App icon texture
    LTexture *texture = nullptr;

    // Name texture (for topbar)
    LTexture *nameTexture = nullptr;

    // App icon view on each Output dock
    std::list<DockApp*>dockApps;
    LPoint dockAppsAnimationOffset;
    LAnimation *launchAnimation = nullptr;
};

#endif // APP_H
