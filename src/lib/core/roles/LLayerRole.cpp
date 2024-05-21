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
    m_namespace(static_cast<const LLayerRole::Params*>(params)->nameSpace)
{
    setOutput(static_cast<const LLayerRole::Params*>(params)->output);
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

    if (anchor() == (LEdgeTop | LEdgeLeft))
    {
        m_rolePos = output()->pos() + exclusiveZone().rect().pos();
    }
    else if (anchor() == (LEdgeTop | LEdgeRight))
    {
        m_rolePos.setX(output()->pos().x() + exclusiveZone().rect().x() + exclusiveZone().rect().w() - surface()->size().w());
        m_rolePos.setY(output()->pos().y() + exclusiveZone().rect().y());
    }
    else if (anchor() == (LEdgeBottom | LEdgeRight))
    {
        m_rolePos.setX(output()->pos().x() + exclusiveZone().rect().x() + exclusiveZone().rect().w() - surface()->size().w());
        m_rolePos.setY(output()->pos().y() + exclusiveZone().rect().y() + exclusiveZone().rect().h() - surface()->size().h());
    }
    else if (anchor() == (LEdgeBottom | LEdgeLeft))
    {
        m_rolePos.setX(output()->pos().x() + exclusiveZone().rect().x());
        m_rolePos.setY(output()->pos().y() + exclusiveZone().rect().y() + exclusiveZone().rect().h() - surface()->size().h());
    }
    else if (anchor() == LEdgeTop || anchor() == (LEdgeLeft | LEdgeTop | LEdgeRight))
    {
        m_rolePos.setX(output()->pos().x() + exclusiveZone().rect().x() + (exclusiveZone().rect().w() - surface()->size().w()) / 2 );
        m_rolePos.setY(output()->pos().y() + exclusiveZone().rect().y());
    }
    else if (anchor() == LEdgeBottom || anchor() == (LEdgeLeft | LEdgeBottom | LEdgeRight))
    {
        m_rolePos.setX(output()->pos().x() + exclusiveZone().rect().x() + (exclusiveZone().rect().w() - surface()->size().w()) / 2 );
        m_rolePos.setY(output()->pos().y() + exclusiveZone().rect().y() + exclusiveZone().rect().h() - surface()->size().h());
    }
    else if (anchor() == LEdgeLeft || anchor() == (LEdgeBottom | LEdgeLeft | LEdgeTop))
    {
        m_rolePos.setX(output()->pos().x() + exclusiveZone().rect().x());
        m_rolePos.setY(output()->pos().y() + exclusiveZone().rect().y() + (exclusiveZone().rect().h() - surface()->size().h()) / 2);
    }
    else if (anchor() == LEdgeRight || anchor() == (LEdgeBottom | LEdgeRight | LEdgeTop))
    {
        m_rolePos.setX(output()->pos().x() + exclusiveZone().rect().x() + exclusiveZone().rect().w() - surface()->size().w());
        m_rolePos.setY(output()->pos().y() + exclusiveZone().rect().y() + (exclusiveZone().rect().h() - surface()->size().h()) / 2);
    }
    // Center
    else
    {
        m_rolePos = output()->pos() + exclusiveZone().rect().pos() + (exclusiveZone().rect().size() - surface()->size()) / 2;
    }

    // Margin

    if (anchor().check(LEdgeTop))
        m_rolePos.setY(m_rolePos.y() + margin().top);
    if (anchor().check(LEdgeBottom))
        m_rolePos.setY(m_rolePos.y() - margin().bottom);
    if (anchor().check(LEdgeLeft))
        m_rolePos.setX(m_rolePos.x() + margin().left);
    if (anchor().check(LEdgeRight))
        m_rolePos.setX(m_rolePos.x() - margin().right);

    return m_rolePos;
}

void LLayerRole::configureSize(const LSize &size) noexcept
{
    auto &res { *static_cast<LayerShell::RLayerSurface*>(resource()) };
    res.configure(LTime::nextSerial(), size);
}

LEdge LLayerRole::edgesToSingleEdge() const noexcept
{
    if (anchor() == (LEdgeTop | LEdgeLeft))
    {
        if (exclusiveEdge())
            return exclusiveEdge();
        else
            return LEdgeTop;
    }
    else if (anchor() == (LEdgeTop | LEdgeRight))
    {
        if (exclusiveEdge())
            return exclusiveEdge();
        else
            return LEdgeTop;
    }
    else if (anchor() == (LEdgeBottom | LEdgeRight))
    {
        if (exclusiveEdge())
            return exclusiveEdge();
        else
            return LEdgeBottom;
    }
    else if (anchor() == (LEdgeBottom | LEdgeLeft))
    {
        if (exclusiveEdge())
            return exclusiveEdge();
        else
            return LEdgeBottom;
    }
    else if (anchor() == LEdgeTop || anchor() == (LEdgeLeft | LEdgeTop | LEdgeRight))
    {
        return LEdgeTop;
    }
    else if (anchor() == LEdgeBottom || anchor() == (LEdgeLeft | LEdgeBottom | LEdgeRight))
    {
        return LEdgeBottom;
    }
    else if (anchor() == LEdgeLeft || anchor() == (LEdgeBottom | LEdgeLeft | LEdgeTop))
    {
        return LEdgeLeft;
    }
    else if (anchor() == LEdgeRight || anchor() == (LEdgeBottom | LEdgeRight | LEdgeTop))
    {
        return LEdgeRight;
    }
    // Center
    else
    {
        return LEdgeNone;
    }
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
            if (pendingProps().anchor.checkAll(LEdgeLeft | LEdgeRight) == 0)
            {
                wl_resource_post_error(res.resource(),
                                       ZWLR_LAYER_SURFACE_V1_ERROR_INVALID_SIZE,
                                       "width is 0 but anchors do not include left and right");
                return;
            }

            needsConfigure = true;
        }

        if (pendingProps().size.h() == 0)
        {
            if (pendingProps().anchor.checkAll(LEdgeTop | LEdgeBottom) == 0)
            {
                wl_resource_post_error(res.resource(),
                                       ZWLR_LAYER_SURFACE_V1_ERROR_INVALID_SIZE,
                                       "height is 0 but anchors do not include top and bottom");
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

    // Update exclusive zone
    if (changesToNotify.check(AnchorChanged | ExclusiveEdgeChanged | ExclusiveZoneChanged | MarginChanged))
    {
        const LEdge edge { edgesToSingleEdge() };
        Int32 zoneSize { exclusiveZoneSize() };

        if (zoneSize > 0)
        {
            switch (edge)
            {
            case LEdgeTop:
                zoneSize += margin().top;
                break;
            case LEdgeLeft:
                zoneSize += margin().left;
                break;
            case LEdgeRight:
                zoneSize += margin().right;
                break;
            case LEdgeBottom:
                zoneSize += margin().bottom;
                break;
            case LEdgeNone:
                break;
            }
        }

        m_exclusiveZone.setEdgeAndSize(edge, zoneSize);
    }

    if (changesToNotify != 0)
        atomicPropsChanged(changesToNotify, pendingProps());

    /* Sync double buffered state */
    if (changesToNotify.check(AnchorChanged))
        pendingProps().anchor = currentProps().anchor;
    if (changesToNotify.check(ExclusiveEdgeChanged))
        pendingProps().exclusiveEdge = currentProps().exclusiveEdge;
    if (changesToNotify.check(ExclusiveZoneChanged))
        pendingProps().exclusiveZone = currentProps().exclusiveZone;
    if (changesToNotify.check(KeyboardInteractivityChanged))
        pendingProps().keyboardInteractivity = currentProps().keyboardInteractivity;
    if (changesToNotify.check(LayerChanged))
        pendingProps().layer = currentProps().layer;
    if (changesToNotify.check(MarginChanged))
        pendingProps().margin = currentProps().margin;
    if (changesToNotify.check(SizeChanged))
        pendingProps().size = currentProps().size;

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
