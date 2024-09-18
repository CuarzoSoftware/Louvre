#ifndef LXWINDOWROLEPRIVATE_H
#define LXWINDOWROLEPRIVATE_H

#include <LXWindowRole.h>

using namespace Louvre;

struct LXWindowRole::Params
{
    LResource *xWaylandSurfaceRes;
    LSurface *surface;
    UInt32 winId;
    LPoint pos;
    LSize size;
    bool overrideRedirect;
};

LPRIVATE_CLASS(LXWindowRole)
    LXWindowRole *role;
    UInt64 serial;
    UInt32 winId;
    LPoint pos;
    LSize size;
    bool mapped { false };
    bool overrideRedirect;
    bool withdrawn { true };
    bool minimized { false };
    bool modal { false };
    bool fullscreen { false };
    bool maximizedX { false };
    bool maximizedY { false };
    bool focus { false };
    void setOverrideRedirect(bool override);
    void setWithdrawn(bool withdrawn) noexcept;
    void updateWMState() noexcept;
    void updateNetWMState() noexcept;
};

#endif // LXWINDOWROLEPRIVATE_H
