#ifndef APP_H
#define APP_H

#include <LAnimation.h>
#include <LObject.h>

#include "DockApp.h"

class Client;

using namespace Louvre;

/**
 * Manages applications within the compositor.
 *
 * During initialization, the Compositor loads applications from the apps.list
 * file using the G::loadApps() method. For each entry, an App instance is
 * created, added to the dock as a pinned item, and launched when clicked. The
 * compositor associates the Wayland client with the app's process ID.
 *
 * Apps launched outside the dock (e.g., from a terminal) appear as non-pinned
 * items with a default terminal icon and are removed from the dock upon exit.
 *
 * Each output in the compositor has its own dock, and the dockApps vector holds
 * app icon views for each dock.
 */
class App final : public LObject {
 public:
  enum AppState {
    // Not running
    Dead,

    // Jumping dock icons, waiting for a client with the same pid
    Launching,

    // Gray dot displayed at the bottom of each dock icon
    Running
  };

  // Passing NULL as appExec and iconPath, means it is a non pinned app
  App(const char *appName, const char *appExec, const char *iconPath);
  ~App();

  void dockIconClicked();

  // True for Apps pinned in the Dock, false for non Dock apps
  bool pinned{true};

  // App has not yet started
  AppState state{Dead};

  // App icon texture
  LTexture *texture{nullptr};

  // Name texture (for topbar)
  LTexture *nameTexture{nullptr};

  // App icon view on each dock
  std::vector<DockApp *> dockApps;

  // App jumping icon animation
  LAnimation launchAnimation;
  LPoint dockAppsAnimationOffset;

  Client *client{nullptr};

  bool isWofi{false};

  Int32 pid{-1};
  std::string name;
  std::string exec;
};

#endif  // APP_H
