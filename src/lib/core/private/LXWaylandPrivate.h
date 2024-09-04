#ifndef LXWAYLANDPRIVATE_H
#define LXWAYLANDPRIVATE_H

#include <LXWayland.h>
#include <LWeak.h>

#include <xcb/xcb.h>

using namespace Louvre;

LPRIVATE_CLASS(LXWayland)

    void init();
    bool start();

    int wm_fd[2] {-1, -1};
    int wl_fd[2] {-1, -1};
    int x_fd[2] {-1, -1};
    int xDisplay;

    std::string xDisplayName;

    LXWayland *x;
    LWeak<LClient> client;
    pid_t pid { -1 };
    xcb_connection_t *conn { nullptr };
    xcb_screen_t *screen { nullptr };

    xcb_atom_t Protocols;
    xcb_atom_t DeleteWindow;
    xcb_atom_t NetWmState;
    xcb_atom_t NetWmStateFullscreen;
    xcb_atom_t NetWmWindowTypeDialog;
    xcb_atom_t NetWmWindowTypeUtility;
    xcb_atom_t NetWmWindowTypeSplash;
    xcb_atom_t NetWmWindowType;

    wl_client *wlClient;
    wl_event_source *xInitEventSource;
    bool enableWM { true };
};

#endif // LXWAYLANDPRIVATE_H
