#ifndef LXWAYLANDPRIVATE_H
#define LXWAYLANDPRIVATE_H

#include <LXWayland.h>
#include <LWeak.h>

#include <filesystem>
#include <list>
#include <vector>

#include <xcb/render.h>
#include <xcb/xcb.h>

using namespace Louvre;

enum XCBAtoms
{
    WL_SURFACE_ID,
    WL_SURFACE_SERIAL,
    WM_DELETE_WINDOW,
    WM_PROTOCOLS,
    WM_HINTS,
    WM_NORMAL_HINTS,
    WM_SIZE_HINTS,
    WM_WINDOW_ROLE,
    MOTIF_WM_HINTS,
    UTF8_STRING,
    WM_S0,
    NET_SUPPORTED,
    NET_WM_CM_S0,
    NET_WM_PID,
    NET_WM_NAME,
    NET_WM_STATE,
    NET_WM_STRUT_PARTIAL,
    NET_WM_WINDOW_TYPE,
    WM_TAKE_FOCUS,
    WINDOW,
    NET_ACTIVE_WINDOW,
    NET_WM_MOVERESIZE,
    NET_SUPPORTING_WM_CHECK,
    NET_WM_STATE_FOCUSED,
    NET_WM_STATE_MODAL,
    NET_WM_STATE_FULLSCREEN,
    NET_WM_STATE_MAXIMIZED_VERT,
    NET_WM_STATE_MAXIMIZED_HORZ,
    NET_WM_STATE_HIDDEN,
    NET_WM_PING,
    WM_CHANGE_STATE,
    WM_STATE,
    CLIPBOARD,
    PRIMARY,
    WL_SELECTION,
    TARGETS,
    CLIPBOARD_MANAGER,
    INCR,
    TEXT,
    TIMESTAMP,
    DELETE,
    NET_STARTUP_ID,
    NET_STARTUP_INFO,
    NET_STARTUP_INFO_BEGIN,
    NET_WM_WINDOW_TYPE_NORMAL,
    NET_WM_WINDOW_TYPE_UTILITY,
    NET_WM_WINDOW_TYPE_TOOLTIP,
    NET_WM_WINDOW_TYPE_DND,
    NET_WM_WINDOW_TYPE_DROPDOWN_MENU,
    NET_WM_WINDOW_TYPE_POPUP_MENU,
    NET_WM_WINDOW_TYPE_COMBO,
    NET_WM_WINDOW_TYPE_MENU,
    NET_WM_WINDOW_TYPE_NOTIFICATION,
    NET_WM_WINDOW_TYPE_SPLASH,
    NET_WM_WINDOW_TYPE_DESKTOP,
    DND_SELECTION,
    DND_AWARE,
    DND_STATUS,
    DND_POSITION,
    DND_ENTER,
    DND_LEAVE,
    DND_DROP,
    DND_FINISHED,
    DND_PROXY,
    DND_TYPE_LIST,
    DND_ACTION_MOVE,
    DND_ACTION_COPY,
    DND_ACTION_ASK,
    DND_ACTION_PRIVATE,
    NET_CLIENT_LIST,
    NET_CLIENT_LIST_STACKING,
    NET_WORKAREA,
    ATOM_LAST
};

static const char *const atomsMap[]
{
    "WL_SURFACE_ID",
    "WL_SURFACE_SERIAL",
    "WM_DELETE_WINDOW",
    "WM_PROTOCOLS",
    "WM_HINTS",
    "WM_NORMAL_HINTS",
    "WM_SIZE_HINTS",
    "WM_WINDOW_ROLE",
    "_MOTIF_WM_HINTS",
    "UTF8_STRING",
    "WM_S0",
    "_NET_SUPPORTED",
    "_NET_WM_CM_S0",
    "_NET_WM_PID",
    "_NET_WM_NAME",
    "_NET_WM_STATE",
    "_NET_WM_STRUT_PARTIAL",
    "_NET_WM_WINDOW_TYPE",
    "WM_TAKE_FOCUS",
    "WINDOW",
    "_NET_ACTIVE_WINDOW",
    "_NET_WM_MOVERESIZE",
    "_NET_SUPPORTING_WM_CHECK",
    "_NET_WM_STATE_FOCUSED",
    "_NET_WM_STATE_MODAL",
    "_NET_WM_STATE_FULLSCREEN",
    "_NET_WM_STATE_MAXIMIZED_VERT",
    "_NET_WM_STATE_MAXIMIZED_HORZ",
    "_NET_WM_STATE_HIDDEN",
    "_NET_WM_PING",
    "WM_CHANGE_STATE",
    "WM_STATE",
    "CLIPBOARD",
    "PRIMARY",
    "_WL_SELECTION",
    "TARGETS",
    "CLIPBOARD_MANAGER",
    "INCR",
    "TEXT",
    "TIMESTAMP",
    "DELETE",
    "_NET_STARTUP_ID",
    "_NET_STARTUP_INFO",
    "_NET_STARTUP_INFO_BEGIN",
    "_NET_WM_WINDOW_TYPE_NORMAL",
    "_NET_WM_WINDOW_TYPE_UTILITY",
    "_NET_WM_WINDOW_TYPE_TOOLTIP",
    "_NET_WM_WINDOW_TYPE_DND",
    "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
    "_NET_WM_WINDOW_TYPE_POPUP_MENU",
    "_NET_WM_WINDOW_TYPE_COMBO",
    "_NET_WM_WINDOW_TYPE_MENU",
    "_NET_WM_WINDOW_TYPE_NOTIFICATION",
    "_NET_WM_WINDOW_TYPE_SPLASH",
    "_NET_WM_WINDOW_TYPE_DESKTOP",
    "XdndSelection",
    "XdndAware",
    "XdndStatus",
    "XdndPosition",
    "XdndEnter",
    "XdndLeave",
    "XdndDrop",
    "XdndFinished",
    "XdndProxy",
    "XdndTypeList",
    "XdndActionMove",
    "XdndActionCopy",
    "XdndActionAsk",
    "XdndActionPrivate",
    "_NET_CLIENT_LIST",
    "_NET_CLIENT_LIST_STACKING",
    "_NET_WORKAREA"
};

LPRIVATE_CLASS(LXWayland)

    State state { Uninitialized };

    void init();
    bool initXWaylandPath() noexcept;
    bool initDisplay() noexcept;
    bool initServer();
    void execXWayland(int notifyFd);
    static int onServerReady(int fd, UInt32 mask, void *data);

    void initWM();
    void wmGetResources() noexcept;
    void wmGetVisualAndColormap() noexcept;
    void wmGetRenderFormat() noexcept;
    void wmSetNetActiveWindow(UInt32 winId) noexcept;
    void wmCreateWindow() noexcept;

    void unit();
    void unitXWaylandPath() noexcept;
    void unitDisplay() noexcept;
    void unitServer();

    xcb_void_cookie_t sendEventWithSize(UInt8 propagate, UInt32 winId, UInt32 eventMask, const void *event, UInt32 size);

    void handleCreateNotify(void *event);
    void handleDestroyNotify(void *event);
    void handleConfigureRequest(void *event);
    void handleConfigureNotify(void *event);
    void handleMapRequest(void *event);
    void handleMapNotify(void *event);
    void handleClientMessage(void *event);
    void handleSurfaceSerialMessage(void *event);

    static int x11EventHandler(int fd, UInt32 mask, void *data);

    std::unordered_map<UInt32, LXWindowRole *> windows;
    std::list<LXWindowRole*> windowsStack;

    // Only used for NET_CLIENT_LIST_STACKING updates
    std::vector<UInt32> windowIds;
    void updateNetClientListStacking() noexcept;

    wl_event_source *x11EventSource { nullptr };
    wl_event_source *x11IdleStartEventSource { nullptr };

    int wm_fd[2] {-1, -1};
    int wl_fd[2] {-1, -1};
    int x_fd[2] {-1, -1};
    int display { -1 };
    std::string displayName; // e.g :2

    std::filesystem::path xWaylandFullPath;

    LXWayland *x;
    LWeak<LClient> client;
    pid_t pid { -1 };
    xcb_connection_t *conn { nullptr };
    xcb_screen_t *screen { nullptr };

    wl_event_source *xInitEventSource;

    xcb_atom_t atoms[ATOM_LAST];
    const xcb_query_extension_reply_t *xfixes;
    const xcb_query_extension_reply_t *xres;
    UInt32 xfixesMajor;
    xcb_visualid_t visualId;
    xcb_colormap_t colormap;
    xcb_render_pictformat_t renderFormatId;
    xcb_window_t window;
};

#endif // LXWAYLANDPRIVATE_H
