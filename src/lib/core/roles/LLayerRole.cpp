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
    setExclusiveOutput(static_cast<const LLayerRole::Params*>(params)->output);
    currentAtoms().layer = pendingAtoms().layer = static_cast<const LLayerRole::Params*>(params)->layer;
    surface()->imp()->setLayer(layer());

    m_exclusiveZone.setOnRectChangeCallback([this](auto)
    {
        configureRequest();
    });
}

LLayerRole::~LLayerRole() noexcept
{

}

const LPoint &LLayerRole::rolePos() const
{
    if (!exclusiveOutput())
        return m_rolePos;

    if (anchor() == (LEdgeTop | LEdgeLeft))
    {
        m_rolePos = exclusiveOutput()->pos() + exclusiveZone().rect().pos();
    }
    else if (anchor() == (LEdgeTop | LEdgeRight))
    {
        m_rolePos.setX(exclusiveOutput()->pos().x() + exclusiveZone().rect().x() + exclusiveZone().rect().w() - surface()->size().w());
        m_rolePos.setY(exclusiveOutput()->pos().y() + exclusiveZone().rect().y());
    }
    else if (anchor() == (LEdgeBottom | LEdgeRight))
    {
        m_rolePos.setX(exclusiveOutput()->pos().x() + exclusiveZone().rect().x() + exclusiveZone().rect().w() - surface()->size().w());
        m_rolePos.setY(exclusiveOutput()->pos().y() + exclusiveZone().rect().y() + exclusiveZone().rect().h() - surface()->size().h());
    }
    else if (anchor() == (LEdgeBottom | LEdgeLeft))
    {
        m_rolePos.setX(exclusiveOutput()->pos().x() + exclusiveZone().rect().x());
        m_rolePos.setY(exclusiveOutput()->pos().y() + exclusiveZone().rect().y() + exclusiveZone().rect().h() - surface()->size().h());
    }
    else if (anchor() == LEdgeTop || anchor() == (LEdgeLeft | LEdgeTop | LEdgeRight))
    {
        m_rolePos.setX(exclusiveOutput()->pos().x() + exclusiveZone().rect().x() + (exclusiveZone().rect().w() - surface()->size().w()) / 2 );
        m_rolePos.setY(exclusiveOutput()->pos().y() + exclusiveZone().rect().y());
    }
    else if (anchor() == LEdgeBottom || anchor() == (LEdgeLeft | LEdgeBottom | LEdgeRight))
    {
        m_rolePos.setX(exclusiveOutput()->pos().x() + exclusiveZone().rect().x() + (exclusiveZone().rect().w() - surface()->size().w()) / 2 );
        m_rolePos.setY(exclusiveOutput()->pos().y() + exclusiveZone().rect().y() + exclusiveZone().rect().h() - surface()->size().h());
    }
    else if (anchor() == LEdgeLeft || anchor() == (LEdgeBottom | LEdgeLeft | LEdgeTop))
    {
        m_rolePos.setX(exclusiveOutput()->pos().x() + exclusiveZone().rect().x());
        m_rolePos.setY(exclusiveOutput()->pos().y() + exclusiveZone().rect().y() + (exclusiveZone().rect().h() - surface()->size().h()) / 2);
    }
    else if (anchor() == LEdgeRight || anchor() == (LEdgeBottom | LEdgeRight | LEdgeTop))
    {
        m_rolePos.setX(exclusiveOutput()->pos().x() + exclusiveZone().rect().x() + exclusiveZone().rect().w() - surface()->size().w());
        m_rolePos.setY(exclusiveOutput()->pos().y() + exclusiveZone().rect().y() + (exclusiveZone().rect().h() - surface()->size().h()) / 2);
    }
    // Center
    else
    {
        m_rolePos = exclusiveOutput()->pos() + exclusiveZone().rect().pos() + (exclusiveZone().rect().size() - surface()->size()) / 2;
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
    if (m_flags.check(ClosedSent))
        return;

    auto &res { *static_cast<LayerShell::RLayerSurface*>(resource()) };
    res.configure(LTime::nextSerial(), size);
}

void LLayerRole::close() noexcept
{
    if (m_flags.check(ClosedSent))
        return;

    auto &res { *static_cast<LayerShell::RLayerSurface*>(resource()) };
    m_flags.add(ClosedSent);
    res.closed();
    m_exclusiveZone.setOutput(nullptr);
    surface()->imp()->setMapped(false);
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
    if (m_flags.check(ClosedSent))
        return;

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
        if (pendingAtoms().size.w() == 0)
        {
            if (pendingAtoms().anchor.checkAll(LEdgeLeft | LEdgeRight) == 0)
            {
                wl_resource_post_error(res.resource(),
                                       ZWLR_LAYER_SURFACE_V1_ERROR_INVALID_SIZE,
                                       "width is 0 but anchors do not include left and right");
                return;
            }

            needsConfigure = true;
        }

        if (pendingAtoms().size.h() == 0)
        {
            if (pendingAtoms().anchor.checkAll(LEdgeTop | LEdgeBottom) == 0)
            {
                wl_resource_post_error(res.resource(),
                                       ZWLR_LAYER_SURFACE_V1_ERROR_INVALID_SIZE,
                                       "height is 0 but anchors do not include top and bottom");
                return;
            }

            needsConfigure = true;
        }
    }

    LBitset<AtomsChanges> changesToNotify = m_flags & (
        AnchorChanged | ExclusiveEdgeChanged | ExclusiveZoneChanged |
        KeyboardInteractivityChanged | LayerChanged |
        MarginChanged | SizeChanged);

    if (changesToNotify.check(LayerChanged))
        surface()->imp()->setLayer(pendingAtoms().layer);

    m_flags.remove(changesToNotify);

    m_currentAtomsIndex = 1 - m_currentAtomsIndex;

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

            if (zoneSize < 0)
                zoneSize = 0;
        }

        m_exclusiveZone.setEdgeAndSize(edge, zoneSize);
    }

    if (changesToNotify != 0)
        atomsChanged(changesToNotify, pendingAtoms());

    /* Sync double buffered state */
    if (changesToNotify.check(AnchorChanged))
        pendingAtoms().anchor = currentAtoms().anchor;
    if (changesToNotify.check(ExclusiveEdgeChanged))
        pendingAtoms().exclusiveEdge = currentAtoms().exclusiveEdge;
    if (changesToNotify.check(ExclusiveZoneChanged))
        pendingAtoms().exclusiveZone = currentAtoms().exclusiveZone;
    if (changesToNotify.check(KeyboardInteractivityChanged))
        pendingAtoms().keyboardInteractivity = currentAtoms().keyboardInteractivity;
    if (changesToNotify.check(LayerChanged))
        pendingAtoms().layer = currentAtoms().layer;
    if (changesToNotify.check(MarginChanged))
        pendingAtoms().margin = currentAtoms().margin;
    if (changesToNotify.check(SizeChanged))
        pendingAtoms().size = currentAtoms().size;

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
