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
    delete imp()->params;
    delete m_imp;
}

LCompositor *LClient::compositor() const
{
    return imp()->params->compositor;
}

LSeat *LClient::seat() const
{
    return imp()->params->compositor->seat();
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

const list<Protocols::Wayland::GOutput*> &LClient::outputGlobals() const
{
    return imp()->outputGlobals;
}

Protocols::Wayland::GCompositor *LClient::compositorGlobal() const
{
    return imp()->compositorGlobal;
}

list<Protocols::Wayland::GSeat*> &LClient::seatGlobals() const
{
    return imp()->seatGlobals;
}

Protocols::Wayland::GDataDeviceManager *LClient::dataDeviceManagerGlobal() const
{
    return imp()->dataDeviceManagerGlobal;
}

wl_resource *LClient::touchResource() const
{
    return imp()->touchResource;
}

wl_resource *LClient::xdgWmBaseResource() const
{
    return imp()->xdgWmBaseResource;
}

wl_resource *LClient::xdgDecorationManagerResource() const
{
    return imp()->xdgDecorationManagerResource;
}

