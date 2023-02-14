#include <private/LClientPrivate.h>
#include <private/LDataDevicePrivate.h>
#include <LCompositor.h>
#include <LClient.h>

using namespace Louvre;

LClient::LClient(Params *params)
{
    m_imp = new LClientPrivate();
    imp()->params = params;
    dataDevice().imp()->client = this;
}

LClient::~LClient()
{
    delete m_imp->params;
    delete m_imp;
}

LCompositor *LClient::compositor() const
{
    return m_imp->params->compositor;
}

LSeat *LClient::seat() const
{
    return m_imp->params->compositor->seat();
}

wl_client *LClient::client() const
{
    return m_imp->params->client;
}

LDataDevice &LClient::dataDevice() const
{
    return imp()->dataDevice;
}

const list<LSurface *> &LClient::surfaces() const
{
    return m_imp->surfaces;
}

const list<wl_resource *> &LClient::outputs() const
{
    return m_imp->outputResources;
}

Protocols::Wayland::CompositorGlobal *LClient::compositorGlobal() const
{
    return imp()->compositorGlobal;
}

list<Protocols::Wayland::SeatGlobal*> &LClient::seatGlobals() const
{
    return imp()->seatGlobals;
}

Protocols::Wayland::DataDeviceManagerGlobal *LClient::dataDeviceManagerGlobal() const
{
    return imp()->dataDeviceManagerGlobal;
}

wl_resource *LClient::touchResource() const
{
    return m_imp->touchResource;
}

LClient::LClientPrivate *LClient::imp() const
{
    return m_imp;
}

wl_resource *LClient::xdgWmBaseResource() const
{
    return m_imp->xdgWmBaseResource;
}

wl_resource *LClient::xdgDecorationManagerResource() const
{
    return m_imp->xdgDecorationManagerResource;
}

