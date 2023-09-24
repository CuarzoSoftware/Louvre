#ifndef LTOPLEVELROLEPRIVATE_H
#define LTOPLEVELROLEPRIVATE_H

#include <LToplevelRole.h>

using namespace Louvre;
using namespace Louvre::Protocols::XdgDecoration;

struct LToplevelRole::Params
{
    LResource *toplevel;
    LSurface *surface;
};

LPRIVATE_CLASS(LToplevelRole)
    struct ToplevelConfiguration
    {
        bool commited                                               = false;
        LSize size                                                  = LSize();
        States flags                                                = NoState;
        UInt32 serial                                               = 0;
    };

    LToplevelRole *toplevel;

    States stateFlags = NoState;
    ToplevelConfiguration currentConf;
    std::list<ToplevelConfiguration>sentConfs;

    bool hasPendingMinSize                                          = false;
    bool hasPendingMaxSize                                          = false;

    LSize currentMinSize, pendingMinSize;
    LSize currentMaxSize, pendingMaxSize;

    void setAppId(const char *appId);
    void setTitle(const char *title);
    char *appId = nullptr;
    char *title = nullptr;

    RXdgToplevelDecoration *xdgDecoration                           = nullptr;
    DecorationMode decorationMode                                   = ClientSide;
    UInt32 pendingDecorationMode                                    = ClientSide;
    UInt32 lastDecorationModeConfigureSerial                        = 0;
    LToplevelRole::DecorationMode preferredDecorationMode           = NoPreferredMode;

    void applyPendingChanges();
};

#endif // LTOPLEVELROLEPRIVATE_H
