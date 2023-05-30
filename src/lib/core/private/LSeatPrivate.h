#ifndef LSEATPRIVATE_H
#define LSEATPRIVATE_H

#include <LSeat.h>
#include <private/LDNDManagerPrivate.h>

#ifdef  __cplusplus
extern "C" {
#endif

#include <libseat.h>

#ifdef  __cplusplus
}
#endif

struct LSeat::Params
{
    /* Add here any required constructor param */
};

LPRIVATE_CLASS(LSeat)

    // Wayland
    LPointer *pointer                               = nullptr;
    LKeyboard *keyboard                             = nullptr;

    LToplevelRole *activeToplevel                   = nullptr;
    UInt32 capabilities                             = Pointer | Keyboard;

    // Data device
    LDataSource *dataSelection                      = nullptr;
    LDNDManager *dndManager                         = nullptr;

    void *inputBackendData                          = nullptr;

    libseat *libseatHandle                          = nullptr;
    libseat_seat_listener listener;
    bool enabled                                    = false;

    static int seatEvent(int, unsigned int, void*data);
    static void seatEnabled(libseat *seat, void *data);
    static void seatDisabled(libseat *seat, void *data);
    void dispatchSeat();
};

#endif // LSEATPRIVATE_H
