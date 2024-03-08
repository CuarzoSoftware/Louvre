#ifndef LCLIENTPRIVATE_H
#define LCLIENTPRIVATE_H

#include <LClient.h>
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
    Events events;
    LClientCursor lastCursorRequest;

    // Globals
    std::vector<Wayland::GCompositor*> compositorGlobals;
    std::vector<Wayland::GOutput*> outputGlobals;
    std::vector<Wayland::GSeat*> seatGlobals;
    std::vector<Wayland::GSubcompositor*> subcompositorGlobals;
    std::vector<Wayland::GDataDeviceManager*> dataDeviceManagerGlobals;
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
};

#endif // LCLIENTPRIVATE_H
