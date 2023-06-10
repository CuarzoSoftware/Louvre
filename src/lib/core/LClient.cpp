#include <protocols/XdgShell/GXdgWmBase.h>

#include <private/LClientPrivate.h>
#include <private/LDataDevicePrivate.h>

#include <LCompositor.h>
#include <LClient.h>

LClient::LClient(Params *params)
{
    m_imp = new LClientPrivate();
    imp()->params = params;
    dataDevice().imp()->client = this;
}

void LClient::ping(UInt32 serial) const
{
    /* If the client is not bound to any xdg_wm_base global call pong
     * immediatelly. (The user has no way to know if the
     * client is frozen anyway). */
    if (imp()->xdgWmBaseGlobals.empty())
        pong(serial);
    else
        imp()->xdgWmBaseGlobals.front()->ping(serial);
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

const list<Wayland::GOutput*> &LClient::outputGlobals() const
{
    return imp()->outputGlobals;
}

const Wayland::GCompositor *LClient::compositorGlobal() const
{
    return imp()->compositorGlobal;
}

const list<Wayland::GSubcompositor *> &LClient::subcompositorGlobals() const
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
