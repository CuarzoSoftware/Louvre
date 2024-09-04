#include <private/LXWaylandPrivate.h>
#include <iostream>
#include <ostream>
#include <cassert>
#include <unistd.h>

using namespace Louvre;

LXWayland::LXWayland(const void *params) noexcept :
    LFactoryObject(FactoryObjectType),
    LPRIVATE_INIT_UNIQUE(LXWayland)
{
    assert(params != nullptr && "Invalid parameter passed to LXWayland constructor.");
    LXWayland**ptr { (LXWayland**) params };
    assert(*ptr == nullptr && *ptr == xWayland() && "Only a single LXWayland instance can exist.");
    *ptr = this;
    imp()->x = this;
    imp()->init();
}

LXWayland::~LXWayland()
{

}

/*
static xcb_atom_t GetAtom(LXWayland *x, std::string AtomName)
{
    xcb_intern_atom_reply_t *Atom = xcb_intern_atom_reply(x->conn, xcb_intern_atom(x->conn, 0, AtomName.length(), AtomName.c_str()), nullptr);
    if (!Atom) {
        std::cerr << "Failed to get Atom: " << AtomName << " [EXIT]" << std::endl;
    }
    xcb_atom_t ReturnAtom = Atom->atom;
    free(Atom);
    return ReturnAtom;
}
*/

bool LXWayland::start()
{
    return imp()->start();
}

void LXWayland::started()
{
    /*
    usleep(500000);

    // Create a connection
    conn = xcb_connect(":10", nullptr);

    if (xcb_connection_has_error(conn))
    {
        std::cerr << "Failed to open the XCB connection!" << std::endl;
        return;
    }

    std::cout << "XCB connection! " << getenv("LOUVRE_WAYLAND_DISPLAY") << std::endl;

    // Create a screen
    screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;

    if (!screen)
    {
        std::cerr << "Failed to get the XCB screen!" << std::endl;
        return;
    }

    Protocols = GetAtom(this, "WM_PROTOCOLS");
    DeleteWindow = GetAtom(this, "WM_DELETE_WINDOW");
    NetWmState = GetAtom(this, "_NET_WM_STATE");
    NetWmStateFullscreen = GetAtom(this, "_NET_WM_STATE_FULLSCREEN");
    NetWmWindowTypeDialog = GetAtom(this, "_NET_WM_WINDOW_TYPE_DIALOG");
    NetWmWindowTypeUtility = GetAtom(this, "_NET_WM_WINDOW_TYPE_UTILITY");
    NetWmWindowTypeSplash = GetAtom(this, "_NET_WM_WINDOW_TYPE_SPLASH");
    NetWmWindowType = GetAtom(this, "_NET_WM_WINDOW_TYPE");
    */
}
