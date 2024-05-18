#include <protocols/LayerShell/wlr-layer-shell-unstable-v1.h>
#include <private/LLayerRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LSurfacePrivate.h>
#include <protocols/LayerShell/RLayerSurface.h>
#include <LTime.h>

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
    if (!output())
        return m_rolePos;

    if (anchor() == (Top | Left))
    {
        m_rolePos = output()->pos();
    }
    else if (anchor() == (Top | Right))
    {
        m_rolePos.setX(output()->pos().x() + output()->size().w() - size().w());
        m_rolePos.setY(output()->pos().y());
    }

    return m_rolePos;
}

void LLayerRole::configureSize(const LSize &size) noexcept
{
    auto &res { *static_cast<LayerShell::RLayerSurface*>(resource()) };
    res.configure(LTime::nextSerial(), size);
}

void LLayerRole::handleSurfaceCommit(CommitOrigin /*origin*/) noexcept
{
    auto &res { *static_cast<LayerShell::RLayerSurface*>(resource()) };

    if (m_flags.check(Flags::HasPendingInitialConf) && surface()->buffer())
    {
        wl_resource_post_error(res.resource(),
                               ZWLR_LAYER_SHELL_V1_ERROR_ALREADY_CONSTRUCTED,
                               "wl_surface has a buffer attached or committed");
        return;
    }

    bool needsConfigure { false };

    if (m_flags.check(Flags::HasPendingSize))
    {
        if (pendingProps().size.w() == 0)
        {
            if (pendingProps().anchor.check(Edge::Top | Edge::Bottom) == 0)
            {
                wl_resource_post_error(res.resource(),
                                       ZWLR_LAYER_SURFACE_V1_ERROR_INVALID_SIZE,
                                       "width is 0 but anchors are not top or bottom");
                return;
            }

            needsConfigure = true;
        }

        if (pendingProps().size.h() == 0)
        {
            if (pendingProps().anchor.check(Edge::Left | Edge::Right) == 0)
            {
                wl_resource_post_error(res.resource(),
                                       ZWLR_LAYER_SURFACE_V1_ERROR_INVALID_SIZE,
                                       "height is 0 but anchors are not left or right");
                return;
            }

            needsConfigure = true;
        }
    }

    LBitset<AtomicPropChanges> changesToNotify = m_flags & (
        AnchorChanged | ExclusiveEdgeChanged | ExclusiveZoneChanged |
        KeyboardInteractivityChanged | LayerChanged |
        MarginChanged | SizeChanged);

    if (changesToNotify.check(LayerChanged))
        surface()->imp()->setLayer(pendingProps().layer);

    m_flags.remove(changesToNotify);

    m_currentAtomicPropsIndex = 1 - m_currentAtomicPropsIndex;

    if (changesToNotify != 0)
        atomicPropsChanged(changesToNotify, pendingProps());

    if (surface()->mapped())
    {
        if (!surface()->buffer())
        {
            surface()->imp()->setMapped(false);
            m_flags.add(Flags::HasPendingInitialConf);
        }
    }
    else
    {
        if (surface()->buffer())
            surface()->imp()->setMapped(true);
        else
        {
            m_flags.remove(Flags::HasPendingInitialConf);
            needsConfigure = true;
        }
    }

    if (needsConfigure)
        configureRequest();
}
