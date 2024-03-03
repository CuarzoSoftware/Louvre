#ifndef LCLIENTPRIVATE_H
#define LCLIENTPRIVATE_H

#include <LClient.h>
#include <LDataDevice.h>
#include <LClientCursor.h>

using namespace Louvre;
using namespace Louvre::Protocols;

struct LClient::Params
{
    wl_client *client;
};

class LClient::LClientPrivate
{
public:
    LClientPrivate(LClient *lClient, wl_client *wlClient) noexcept :
        client {wlClient},
        lastCursorRequest {lClient}
    {}
    ~LClientPrivate() noexcept = default;
    LClientPrivate(const LClientPrivate&) = delete;
    LClientPrivate &operator=(const LClientPrivate&) = delete;

    wl_client *client;
    LDataDevice dataDevice;
    std::vector<LSurface*> surfaces;
    Events events;
    LClientCursor lastCursorRequest;

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
    std::vector<GammaControl::GGammaControlManager*> gammaControlManagerGlobals;
    std::vector<TearingControl::GTearingControlManager*> tearingControlManagerGlobals;
    std::vector<RelativePointer::GRelativePointerManager*> relativePointerManagerGlobals;
    std::vector<PointerGestures::GPointerGestures*> pointerGesturesGlobals;

    // Singleton Globals
    Wayland::GDataDeviceManager *dataDeviceManagerGlobal { nullptr };
};

#endif // LCLIENTPRIVATE_H
