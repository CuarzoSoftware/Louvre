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

struct LSeat::Params
{
    /* Add here any required constructor param */
};

LPRIVATE_CLASS(LSeat)

    Int32 ttyNumber                         { -1 };

    LToplevelRole *activeToplevel           { nullptr };
    InputCapabilitiesFlags capabilities     { Pointer | Keyboard | Touch };

    std::vector<LToplevelResizeSession*> resizeSessions;
    std::vector<LToplevelMoveSession*> moveSessions;

    void *inputBackendData                  { nullptr };

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
