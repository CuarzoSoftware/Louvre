#include <private/LLayerRolePrivate.h>
#include <private/LSurfacePrivate.h>

using namespace Louvre;

LLayerRole::LLayerRole(const void *params) noexcept :
    LBaseSurfaceRole(FactoryObjectType,
        static_cast<const LLayerRole::Params*>(params)->layerSurfaceRes,
        static_cast<const LLayerRole::Params*>(params)->surface,
        LSurface::Role::Layer),
    m_output(static_cast<const LLayerRole::Params*>(params)->output),
    m_namespace(static_cast<const LLayerRole::Params*>(params)->nameSpace)
{
    currentProps().layer = pendingProps().layer = static_cast<const LLayerRole::Params*>(params)->layer;
    surface()->imp()->setLayer(layer());
}

LLayerRole::~LLayerRole() noexcept
{

}

const LPoint &LLayerRole::rolePos() const
{
    if (output())
        m_rolePos = output()->pos();

    return m_rolePos;
}

void LLayerRole::configureRequest()
{

}
