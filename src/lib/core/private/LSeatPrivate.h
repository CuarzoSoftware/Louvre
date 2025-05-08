#ifndef LSEATPRIVATE_H
#define LSEATPRIVATE_H

#include <LSeat.h>
#include <LDND.h>

#ifdef  __cplusplus
extern "C" {
#endif

#include <libseat.h>

#ifdef  __cplusplus
}
#endif

using namespace Louvre;

LPRIVATE_CLASS(LSeat)

    Int32 ttyNumber                         { -1 };

    LToplevelRole *activeToplevel           { nullptr };

    std::vector<LToplevelResizeSession*> resizeSessions;
    std::vector<LToplevelMoveSession*> moveSessions;
    std::vector<LSurface*> idleInhibitors;
    std::vector<const LIdleListener*> idleListeners;
    bool isUserIdleHint                     { false };

    wl_event_source *libseatEventSource     { nullptr };
    libseat *libseatHandle                  { nullptr };
    libseat_seat_listener listener;
    bool enabled                            { false };

    bool initLibseat();
    static void seatEnabled(libseat *seat, void *data);
    static void seatDisabled(libseat *seat, void *data);
    void dispatchSeat();

    void backendOutputPlugged(LOutput *output);
    void backendOutputUnplugged(LOutput *output);
};

#endif // LSEATPRIVATE_H
