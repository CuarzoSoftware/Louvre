#ifndef LSEATPRIVATE_H
#define LSEATPRIVATE_H

#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/Seat/LDND.h>
#include <CZ/Core/CZEventSource.h>

#ifdef  __cplusplus
extern "C" {
#endif

#include <libseat.h>

#ifdef  __cplusplus
}
#endif

using namespace CZ;

LPRIVATE_CLASS(LSeat)

    std::shared_ptr<CZEventSource> libseatEventSource;
    Int32 ttyNumber                         { -1 };

    CZWeak<LToplevelRole> activeToplevelRole;

    std::vector<LToplevelResizeSession*> resizeSessions;
    std::vector<LToplevelMoveSession*> moveSessions;
    std::vector<LSurface*> idleInhibitors;
    std::vector<const LIdleListener*> idleListeners;
    bool isUserIdleHint                     { false };

    libseat *libseatHandle                  { nullptr };
    libseat_seat_listener listener;
    bool enabled                            { false };

    bool initLibseat();
    static void seatEnabled(libseat *seat, void *data);
    static void seatDisabled(libseat *seat, void *data);
    void dispatchSeat();

    void handleOutputPlugged(LOutput *output) noexcept;
    void handleOutputUnplugged(LOutput *output) noexcept;
    void setActiveToplevel(LToplevelRole *newToplevel) noexcept;
};

#endif // LSEATPRIVATE_H
