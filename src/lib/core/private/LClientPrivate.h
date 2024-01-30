#ifndef LCLIENTPRIVATE_H
#define LCLIENTPRIVATE_H

#include <LClient.h>
#include <LDataDevice.h>

using namespace Louvre;
using namespace Louvre::Protocols;

struct LClient::Params
{
    wl_client *client;
};

LPRIVATE_CLASS(LClient)

    wl_client *client = nullptr;
    LDataDevice dataDevice;
    std::vector<LSurface*> surfaces;

    // Globals
    std::vector<Wayland::GCompositor*> compositorGlobals;
    std::vector<Wayland::GOutput*> outputGlobals;
    std::vector<Wayland::GSeat*> seatGlobals;
    std::vector<Wayland::GSubcompositor*> subcompositorGlobals;
    std::vector<XdgShell::GXdgWmBase*> xdgWmBaseGlobals;
    std::vector<XdgDecoration::GXdgDecorationManager*> xdgDecorationManagerGlobals;
    std::vector<WpPresentationTime::GWpPresentation*> wpPresentationTimeGlobals;
    std::vector<LinuxDMABuf::GLinuxDMABuf*> linuxDMABufGlobals;
    std::vector<Viewporter::GViewporter*> viewporterGlobals;
    std::vector<FractionalScale::GFractionalScaleManager*> fractionalScaleManagerGlobals;

    // Singleton Globals
    Wayland::GDataDeviceManager *dataDeviceManagerGlobal = nullptr;
};

#endif // LCLIENTPRIVATE_H
