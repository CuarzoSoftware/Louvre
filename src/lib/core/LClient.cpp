#include <protocols/XdgShell/GXdgWmBase.h>
#include <private/LDataDevicePrivate.h>
#include <private/LClientPrivate.h>
#include <LCompositor.h>
#include <LClient.h>

using namespace std;

LClient::LClient(Params *params)
{
    m_imp = new LClientPrivate();
    imp()->params = params;
    dataDevice().imp()->client = this;
}

bool LClient::ping(UInt32 serial) const
{
    if (imp()->xdgWmBaseGlobals.empty())
        return false;

    imp()->xdgWmBaseGlobals.front()->ping(serial);
    return true;
}

LClient::~LClient()
{
    delete imp()->params;
    delete m_imp;
}

wl_client *LClient::client() const
{
    return imp()->params->client;
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

const list<XdgDecoration::GXdgDecorationManager *> &LClient::xdgDecorationManagerGlobals() const
{
    return imp()->xdgDecorationManagerGlobals;
}
