#ifndef LTOPLEVELROLEPRIVATE_H
#define LTOPLEVELROLEPRIVATE_H

#include <LToplevelRole.h>

struct Louvre::LToplevelRole::Params
{
    wl_resource *toplevel;
    LSurface *surface;
};

class Louvre::LToplevelRole::LToplevelRolePrivate
{
public:
    LToplevelRolePrivate()                                          = default;
    ~LToplevelRolePrivate()                                         = default;

    LToplevelRolePrivate(const LToplevelRolePrivate&)               = delete;
    LToplevelRolePrivate& operator= (const LToplevelRolePrivate&)   = delete;

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

    #if LOUVRE_XDG_WM_BASE_VERSION >= 4
    LSize boundsC, boundsS;
    #endif

    #if LOUVRE_XDG_WM_BASE_VERSION >= 5
    UChar8 wmCapabilities                                           = 0;
    #endif

    wl_resource *xdgDecoration                                      = nullptr;
    DecorationMode decorationMode                                   = ClientSide;
    UInt32 pendingDecorationMode                                    = ClientSide;
    UInt32 lastDecorationModeConfigureSerial                        = 0;

};

#endif // LTOPLEVELROLEPRIVATE_H
