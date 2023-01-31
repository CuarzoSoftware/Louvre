#ifndef LCLIENTPRIVATE_H
#define LCLIENTPRIVATE_H

#include <LClient.h>

struct Louvre::LClient::Params
{
    wl_client *client;
    LCompositor *compositor;
};

class Louvre::LClient::LClientPrivate
{
public:
    LClientPrivate()                                    = default;
    ~LClientPrivate()                                   = default;

    LClientPrivate(const LClientPrivate&)               = delete;
    LClientPrivate &operator=(const LClientPrivate&)    = delete;

    std::list<LSurface*> surfaces;

    LClient::Params     *params                         = nullptr;
    LDataDevice         *dataDevice                     = nullptr;

    list<wl_resource*>   outputResources;
    list<LWaylandSeatGlobal*> seatGlobals;
    wl_resource         *compositorResource             = nullptr;
    wl_resource         *touchResource                  = nullptr;
    wl_resource         *xdgWmBaseResource              = nullptr;
    wl_resource         *xdgDecorationManagerResource   = nullptr;
    wl_resource         *linuxDMABufResource            = nullptr;
    wl_resource         *presentationTimeResource       = nullptr;
};



#endif // LCLIENTPRIVATE_H
