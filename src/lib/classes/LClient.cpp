#include <private/LClientPrivate.h>

#include <LCompositor.h>
#include <LClient.h>

using namespace Louvre;

LClient::LClient(Params *params)
{
    m_imp = new LClientPrivate();
    m_imp->params = params;
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

LDataDevice *LClient::dataDevice() const
{
    return m_imp->dataDevice;
}

const list<LSurface *> &LClient::surfaces() const
{
    return m_imp->surfaces;
}

const list<wl_resource *> &LClient::outputs() const
{
    return m_imp->outputResources;
}

wl_resource *LClient::compositorResource() const
{
    return m_imp->compositorResource;
}

wl_resource *LClient::seatResource() const
{
    return m_imp->seatResource;
}

wl_resource *LClient::pointerResource() const
{
    return m_imp->pointerResource;
}

wl_resource *LClient::keyboardResource() const
{
    return m_imp->keyboardResource;
}

wl_resource *LClient::touchResource() const
{
    return m_imp->touchResource;
}

const LClient::Serials &LClient::serials() const
{
    return m_imp->serials;
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

