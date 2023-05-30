#ifndef LTOPLEVELROLEPRIVATE_H
#define LTOPLEVELROLEPRIVATE_H

#include <LToplevelRole.h>

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
        LSize sizeS                                                 = LSize();
        States flags                                                = NoState;
        UInt32 serial                                               = 0;
    };

    LToplevelRole *toplevel;

    States stateFlags = NoState;
    ToplevelConfiguration currentConf;
    std::list<ToplevelConfiguration>sentConfs;

    void dispachLastConfiguration();

    bool hasPendingMinSize                                          = false;
    bool hasPendingMaxSize                                          = false;
    bool hasPendingWindowGeometry                                   = false;
    bool windowGeometrySet                                          = false;

    LSize currentMinSizeS, pendingMinSizeS;
    LSize currentMaxSizeS, pendingMaxSizeS;
    LSize currentMinSizeC;
    LSize currentMaxSizeC;
    LRect currentWindowGeometryS, pendingWindowGeometryS;
    LRect currentWindowGeometryC;

    void setAppId(const char *appId);
    void setTitle(const char *title);
    char *appId = nullptr;
    char *title = nullptr;

    // Since 4
    LSize boundsC, boundsS;

    // Since 5
    UChar8 wmCapabilities                                           = 0;

    RXdgToplevelDecoration *xdgDecoration                           = nullptr;
    DecorationMode decorationMode                                   = ClientSide;
    UInt32 pendingDecorationMode                                    = ClientSide;
    UInt32 lastDecorationModeConfigureSerial                        = 0;
};

#endif // LTOPLEVELROLEPRIVATE_H
