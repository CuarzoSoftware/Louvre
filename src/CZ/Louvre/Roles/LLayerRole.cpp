#include <CZ/Louvre/Protocols/LayerShell/wlr-layer-shell-unstable-v1.h>
#include <CZ/Louvre/Private/LLayerRolePrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Protocols/LayerShell/RLayerSurface.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Core/CZTime.h>

using namespace CZ;

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
    m_current.layer = m_pending.layer = m_initLayer;

    m_exclusiveZone.setOnRectChangeCallback([this](auto)
    {
        if (surface()->mapped() && surface()->bufferResource())
            configureRequest();
    });
}

LLayerRole::~LLayerRole() noexcept
{
    validateDestructor();
    notifyDestruction();
}

void LLayerRole::configureSize(const SkISize &size) noexcept
{
    if (m_flags.has(ClosedSent))
        return;

    auto &res { *static_cast<LayerShell::RLayerSurface*>(resource()) };
    res.configure(CZTime::NextSerial(), size);
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

CZEdge LLayerRole::edgesToSingleEdge() const noexcept
{
    if (anchor() == (CZEdgeTop | CZEdgeLeft))
    {
        if (exclusiveEdge())
            return exclusiveEdge();
        else
            return CZEdgeTop;
    }
    else if (anchor() == (CZEdgeTop | CZEdgeRight))
    {
        if (exclusiveEdge())
            return exclusiveEdge();
        else
            return CZEdgeTop;
    }
    else if (anchor() == (CZEdgeBottom | CZEdgeRight))
    {
        if (exclusiveEdge())
            return exclusiveEdge();
        else
            return CZEdgeBottom;
    }
    else if (anchor() == (CZEdgeBottom | CZEdgeLeft))
    {
        if (exclusiveEdge())
            return exclusiveEdge();
        else
            return CZEdgeBottom;
    }
    else if (anchor() == CZEdgeTop || anchor() == (CZEdgeLeft | CZEdgeTop | CZEdgeRight))
    {
        return CZEdgeTop;
    }
    else if (anchor() == CZEdgeBottom || anchor() == (CZEdgeLeft | CZEdgeBottom | CZEdgeRight))
    {
        return CZEdgeBottom;
    }
    else if (anchor() == CZEdgeLeft || anchor() == (CZEdgeBottom | CZEdgeLeft | CZEdgeTop))
    {
        return CZEdgeLeft;
    }
    else if (anchor() == CZEdgeRight || anchor() == (CZEdgeBottom | CZEdgeRight | CZEdgeTop))
    {
        return CZEdgeRight;
    }
    // Center
    else
    {
        return CZEdgeNone;
    }
}

void LLayerRole::cacheCommit() noexcept
{
    if (surface()->isLocked())
        m_cache.emplace_back(std::shared_ptr<State>(new State(m_pending)));
}

void LLayerRole::applyCommit() noexcept
{
    std::shared_ptr<State> pendingCache;
    State *pending;

    if (m_cache.empty())
        pending = &m_pending;
    else
    {
        pendingCache = m_cache.front();
        pending = pendingCache.get();
        m_cache.pop_front();
    }

    if (m_flags.has(ClosedSent))
        return;

    // Unmap request
    if (m_flags.has(MappedByClient) && !surface()->bufferResource())
    {
        m_flags.remove(MappedByClient);
        updateMappingState();

        // Reset conf
        m_current = *pending = {};
        m_flags = HasPendingInitialConf;
        m_exclusiveZone.setSize(0);
        setExclusiveOutput(m_initOutput);
        m_current.layer = m_pending.layer = m_initLayer;
        return;
    }

    auto &res { *static_cast<LayerShell::RLayerSurface*>(resource()) };

    if (m_flags.has(Flags::HasPendingInitialConf) && surface()->bufferResource())
    {
        res.postError(
            ZWLR_LAYER_SHELL_V1_ERROR_ALREADY_CONSTRUCTED,
            "wl_surface has a buffer attached or committed");
        return;
    }

    bool needsConfigure { pending->requestedConfigure };
    pending->requestedConfigure = false;

    CZBitset<Changes> changesToNotify;

    if ( m_current.size != pending->size)
    {
        changesToNotify.add(SizeChanged);

        if (pending->size.width() == 0)
        {
            if (pending->anchor.hasAll(CZEdgeLeft | CZEdgeRight) == 0)
            {
                res.postError(
                    ZWLR_LAYER_SURFACE_V1_ERROR_INVALID_SIZE,
                    "width is 0 but anchors do not include left and right (anchor must be set to opposite edges in the omitted dimensions)");
                return;
            }
        }

        if (pending->size.height() == 0)
        {
            if (pending->anchor.hasAll(CZEdgeTop | CZEdgeBottom) == 0)
            {
                res.postError(
                    ZWLR_LAYER_SURFACE_V1_ERROR_INVALID_SIZE,
                    "height is 0 but anchors do not include top and bottom (anchor must be set to opposite edges in the omitted dimensions)");
                return;
            }
        }
    }

    changesToNotify.setFlag(AnchorChanged, m_current.anchor != pending->anchor);
    changesToNotify.setFlag(ExclusiveEdgeChanged, m_current.exclusiveEdge != pending->exclusiveEdge);
    changesToNotify.setFlag(ExclusiveZoneSizeChanged, m_current.exclusiveZoneSize != pending->exclusiveZoneSize);
    changesToNotify.setFlag(MarginsChanged, m_current.margins != pending->margins);
    changesToNotify.setFlag(KeyboardInteractivityChanged, m_current.keyboardInteractivity != pending->keyboardInteractivity);

    if (m_current.layer != pending->layer)
    {
        changesToNotify.add(LayerChanged);
        surface()->imp()->setLayer(pending->layer);
    }

    if (changesToNotify != 0)
    {
        const auto prev { m_current };
        m_current = *pending;

        // Update exclusive zone
        if (changesToNotify.has(AnchorChanged | ExclusiveEdgeChanged | ExclusiveZoneSizeChanged | MarginsChanged))
        {
            const CZEdge edge { edgesToSingleEdge() };
            Int32 zoneSize { exclusiveZoneSize() };

            if (zoneSize > 0 && EdgeIsCorner(anchor()))
            {
                if (exclusiveEdge() == CZEdgeNone)
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
                case CZEdgeTop:
                    zoneSize += margins().top;
                    break;
                case CZEdgeLeft:
                    zoneSize += margins().left;
                    break;
                case CZEdgeRight:
                    zoneSize += margins().right;
                    break;
                case CZEdgeBottom:
                    zoneSize += margins().bottom;
                    break;
                case CZEdgeNone:
                    break;
                }

                if (zoneSize < 0)
                    zoneSize = 0;
            }

            m_exclusiveZone.setEdgeAndSize(edge, zoneSize);
        }

        stateChanged(changesToNotify, prev);
    }

    if (!m_flags.has(MappedByClient))
    {
        if (surface()->bufferResource())
        {
            m_flags.add(MappedByClient);
            updateMappingState();
        }
        else
        {
            m_flags.remove(HasPendingInitialConf);
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
        surface()->bufferResource());
}
