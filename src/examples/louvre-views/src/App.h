#ifndef APP_H
#define APP_H

#include <LObject.h>
#include <LAnimation.h>
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

    App(const char *appName,
        const char *appExec,
        const char *iconPath);
    ~App();

    void clicked();

    Client *client = nullptr;

    Int32 pid = -1;
    std::string name;
    std::string exec;

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

    LAnimation launchAnimation;
};

#endif // APP_H
