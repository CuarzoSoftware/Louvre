#include <protocols/XdgShell/GXdgWmBase.h>
#include <private/LDataDevicePrivate.h>
#include <private/LClientPrivate.h>
#include <LCompositor.h>
#include <LClient.h>

using namespace std;

LClient::LClient(Params *params) : LPRIVATE_INIT_UNIQUE(LClient)
{
    imp()->client = params->client;
    dataDevice().imp()->client = this;
}

LClient::~LClient() {}

bool LClient::ping(UInt32 serial) const
{
    if (imp()->xdgWmBaseGlobals.empty())
        return false;

    imp()->xdgWmBaseGlobals.front()->ping(serial);
    return true;
}

wl_client *LClient::client() const
{
    return imp()->client;
}

LDataDevice &LClient::dataDevice() const
{
    return imp()->dataDevice;
}

const list<LSurface *> &LClient::surfaces() const
{
    return imp()->surfaces;
}

void LClient::flush()
{
    wl_client_flush(client());
}

void LClient::destroy()
{
    wl_client_destroy(client());
}

const list<Wayland::GOutput*> &LClient::outputGlobals() const
{
    return imp()->outputGlobals;
}

const list<Wayland::GCompositor*> &LClient::compositorGlobals() const
{
    return imp()->compositorGlobals;
}

const list<Wayland::GSubcompositor*> &LClient::subcompositorGlobals() const
{
    return imp()->subcompositorGlobals;
}

const list<Wayland::GSeat*> &LClient::seatGlobals() const
{
    return imp()->seatGlobals;
}

const Wayland::GDataDeviceManager *LClient::dataDeviceManagerGlobal() const
{
    return imp()->dataDeviceManagerGlobal;
}

const list<XdgShell::GXdgWmBase *> &LClient::xdgWmBaseGlobals() const
{
    return imp()->xdgWmBaseGlobals;
}

const list<WpPresentationTime::GWpPresentation *> &LClient::wpPresentationTimeGlobals() const
{
    return imp()->wpPresentationTimeGlobals;
}

const list<LinuxDMABuf::GLinuxDMABuf *> &LClient::linuxDMABufGlobals() const
{
    return imp()->linuxDMABufGlobals;
}

const std::list<FractionalScale::GFractionalScaleManager *> &LClient::fractionalScaleManagerGlobals() const
{
    return imp()->fractionalScaleManagerGlobals;
}

const std::list<Viewporter::GViewporter *> &LClient::viewporterGlobals() const
{
    return imp()->viewporterGlobals;
}

const list<XdgDecoration::GXdgDecorationManager *> &LClient::xdgDecorationManagerGlobals() const
{
    return imp()->xdgDecorationManagerGlobals;
}
