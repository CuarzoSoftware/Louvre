#include <CZ/Louvre/Protocols/LayerShell/wlr-layer-shell-unstable-v1.h>
#include <CZ/Louvre/Private/LLayerRolePrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Protocols/LayerShell/RLayerSurface.h>
#include <LTime.h>

using namespace Louvre;

LLayerRole::LLayerRole(const void *params) noexcept :
    LBaseSurfaceRole(FactoryObjectType,
        static_cast<const LLayerRole::Params*>(params)->layerSurfaceRes,
        static_cast<const LLayerRole::Params*>(params)->surface,
        LSurface::Role::Layer),
    m_scope(static_cast<const LLayerRole::Params*>(params)->scope),
    m_initOutput(static_cast<const LLayerRole::Params*>(params)->output),
    m_initLayer(static_cast<const LLayerRole::Params*>(params)->layer)
{
    setExclusiveOutput(m_initOutput);
    currentAtoms().layer = pendingAtoms().layer = m_initLayer;
    surface()->imp()->setLayer(layer());

    m_exclusiveZone.setOnRectChangeCallback([this](auto)
    {
        if (surface()->mapped() && surface()->hasBuffer())
            configureRequest();
    });
}

LLayerRole::~LLayerRole() noexcept
{
    validateDestructor();
    notifyDestruction();
}

void LLayerRole::configureSize(const LSize &size) noexcept
{
    if (m_flags.has(ClosedSent))
        return;

    auto &res { *static_cast<LayerShell::RLayerSurface*>(resource()) };
    res.configure(LTime::nextSerial(), size);

    if (currentAtoms().size.w() == 0)
    {
        currentAtoms().size.setW(size.w());

        if (!m_flags.has(HasPendingSize))
            pendingAtoms().size.setW(size.w());
    }

    if (currentAtoms().size.h() == 0)
    {
        currentAtoms().size.setH(size.h());

        if (!m_flags.has(HasPendingSize))
            pendingAtoms().size.setH(size.h());
    }
}

void LLayerRole::setExclusiveOutput(LOutput *output) noexcept
{
    if (output)
        surface()->sendOutputEnterEvent(output);

    m_exclusiveZone.setOutput(output);
    updateMappingState();
}

void LLayerRole::close() noexcept
{
    if (m_flags.has(ClosedSent))
        return;

    auto &res { *static_cast<LayerShell::RLayerSurface*>(resource()) };
    m_flags.add(ClosedSent);
    res.closed();
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

#include <CZ/Louvre/Private/LOutputPrivate.h>

void LLayerRole::handleSurfaceCommit(CommitOrigin /*origin*/) noexcept
{
    if (m_flags.has(ClosedSent))
        return;

    // Unmap request
    if (m_flags.has(MappedByClient) && !surface()->hasBuffer())
    {
        m_flags.remove(MappedByClient);
        updateMappingState();

        // Reset conf
        m_atoms[0] = Atoms();
        m_atoms[1] = Atoms();
        m_flags = HasPendingInitialConf;
        m_exclusiveZone.setSize(0);
        m_currentAtomsIndex = 0;
        setExclusiveOutput(m_initOutput);
        currentAtoms().layer = pendingAtoms().layer = m_initLayer;
        surface()->imp()->setLayer(m_initLayer);
        return;
    }

    auto &res { *static_cast<LayerShell::RLayerSurface*>(resource()) };

    if (m_flags.has(Flags::HasPendingInitialConf) && surface()->hasBuffer())
    {
        res.postError(
            ZWLR_LAYER_SHELL_V1_ERROR_ALREADY_CONSTRUCTED,
            "wl_surface has a buffer attached or committed");
        return;
    }

    bool needsConfigure { false };

    if (m_flags.has(Flags::HasPendingSize))
    {
        if (pendingAtoms().size.w() == 0)
        {
            if (pendingAtoms().anchor.hasAll(LEdgeLeft | LEdgeRight) == 0)
            {
                res.postError(
                    ZWLR_LAYER_SURFACE_V1_ERROR_INVALID_SIZE,
                    "width is 0 but anchors do not include left and right (anchor must be set to opposite edges in the omitted dimensions)");
                return;
            }
        }

        if (pendingAtoms().size.h() == 0)
        {
            if (pendingAtoms().anchor.hasAll(LEdgeTop | LEdgeBottom) == 0)
            {
                res.postError(
                    ZWLR_LAYER_SURFACE_V1_ERROR_INVALID_SIZE,
                    "height is 0 but anchors do not include top and bottom (anchor must be set to opposite edges in the omitted dimensions)");
                return;
            }
        }

        needsConfigure = true;
    }

    CZBitset<AtomChanges> changesToNotify = m_flags & (
        AnchorChanged | ExclusiveEdgeChanged | ExclusiveZoneSizeChanged |
        KeyboardInteractivityChanged | LayerChanged |
        MarginsChanged | SizeChanged);

    if (changesToNotify.has(LayerChanged))
        surface()->imp()->setLayer(pendingAtoms().layer);

    m_flags.remove(changesToNotify);

    m_currentAtomsIndex = 1 - m_currentAtomsIndex;

    // Update exclusive zone
    if (changesToNotify.has(AnchorChanged | ExclusiveEdgeChanged | ExclusiveZoneSizeChanged | MarginsChanged))
    {
        const LEdge edge { edgesToSingleEdge() };
        Int32 zoneSize { exclusiveZoneSize() };

        if (zoneSize > 0 && edgeIsCorner(anchor()))
        {
            if (exclusiveEdge() == LEdgeNone)
                zoneSize = 0;
            else if ((anchor() & exclusiveEdge()) == 0)
            {
                res.postError(
                    ZWLR_LAYER_SURFACE_V1_ERROR_INVALID_EXCLUSIVE_EDGE,
                    "The exclusive edge is invalid given the surface anchors");
                return;
            }
        }

        if (zoneSize > 0)
        {
            switch (edge)
            {
            case LEdgeTop:
                zoneSize += margins().top;
                break;
            case LEdgeLeft:
                zoneSize += margins().left;
                break;
            case LEdgeRight:
                zoneSize += margins().right;
                break;
            case LEdgeBottom:
                zoneSize += margins().bottom;
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
    if (changesToNotify.has(AnchorChanged))
        pendingAtoms().anchor = currentAtoms().anchor;
    if (changesToNotify.has(ExclusiveEdgeChanged))
        pendingAtoms().exclusiveEdge = currentAtoms().exclusiveEdge;
    if (changesToNotify.has(ExclusiveZoneSizeChanged))
        pendingAtoms().exclusiveZoneSize = currentAtoms().exclusiveZoneSize;
    if (changesToNotify.has(KeyboardInteractivityChanged))
        pendingAtoms().keyboardInteractivity = currentAtoms().keyboardInteractivity;
    if (changesToNotify.has(LayerChanged))
        pendingAtoms().layer = currentAtoms().layer;
    if (changesToNotify.has(MarginsChanged))
        pendingAtoms().margins = currentAtoms().margins;
    if (changesToNotify.has(SizeChanged))
        pendingAtoms().size = currentAtoms().size;

    if (!m_flags.has(MappedByClient))
    {
        if (surface()->hasBuffer())
        {
            m_flags.add(MappedByClient);
            updateMappingState();
        }
        else
        {
            m_flags.remove(Flags::HasPendingInitialConf);
            needsConfigure = true;
        }
    }

    if (needsConfigure)
        configureRequest();
}

void LLayerRole::updateMappingState() noexcept
{
    surface()->imp()->setMapped(
        m_exclusiveZone.output() &&
        m_exclusiveZone.output()->state() != LOutput::State::Uninitialized &&
        m_flags.has(MappedByClient) &&
        surface()->hasBuffer());
}
