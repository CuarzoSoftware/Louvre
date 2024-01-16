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

    void dockIconClicked();

    // True for Apps pinned in the Dock, false for non Dock apps
    bool pinned = true;

    // App has not yet started
    AppState state = Dead;

    // App icon texture
    LTexture *texture = nullptr;

    // Name texture (for topbar)
    LTexture *nameTexture = nullptr;

    // App icon view on each dock
    std::vector<DockApp*>dockApps;

    // App jumping icon animation
    LAnimation launchAnimation;
    LPoint dockAppsAnimationOffset;

    Client *client = nullptr;

    Int32 pid = -1;
    std::string name;
    std::string exec;
};

#endif // APP_H
