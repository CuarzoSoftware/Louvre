#include <CZ/Louvre/Protocols/XdgShell/RXdgSurface.h>
#include <CZ/Louvre/Protocols/XdgShell/RXdgPopup.h>
#include <CZ/Louvre/Protocols/XdgShell/xdg-shell.h>
#include <CZ/Louvre/Private/LPopupRolePrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LPointerPrivate.h>
#include <LPositioner.h>
#include <LCompositor.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LTime.h>
#include <CZ/skia/core/SkRect.h>

using namespace Louvre;

struct Config
{
    const LPopupRole *popup;
    SkIRect finalRect;
    SkIPoint parentPos;
    LPositioner::Gravity gravity;
    LPositioner::Anchor anchor;
    SkIPoint anchorOrigin;
    SkIPoint popupOrigin;
    SkIPoint slide;
};

static void invertAnchor(LPositioner::Anchor &anchor, bool x, bool y) noexcept
{
    if (x)
    {
        if (anchor == LPositioner::AnchorLeft)
            anchor = LPositioner::AnchorRight;
        else if (anchor == LPositioner::AnchorRight)
            anchor = LPositioner::AnchorLeft;
        else if (anchor == LPositioner::AnchorTopLeft)
            anchor = LPositioner::AnchorTopRight;
        else if (anchor == LPositioner::AnchorTopRight)
            anchor = LPositioner::AnchorTopLeft;
        else if (anchor == LPositioner::AnchorBottomLeft)
            anchor = LPositioner::AnchorBottomRight;
        else if (anchor == LPositioner::AnchorBottomRight)
            anchor = LPositioner::AnchorBottomLeft;
    }

    if (y)
    {
        if (anchor == LPositioner::AnchorTop)
            anchor = LPositioner::AnchorBottom;
        else if (anchor == LPositioner::AnchorBottom)
            anchor = LPositioner::AnchorTop;
        else if (anchor == LPositioner::AnchorTopLeft)
            anchor = LPositioner::AnchorBottomLeft;
        else if (anchor == LPositioner::AnchorBottomLeft)
            anchor = LPositioner::AnchorTopLeft;
        else if (anchor == LPositioner::AnchorTopRight)
            anchor = LPositioner::AnchorBottomRight;
        else if (anchor == LPositioner::AnchorBottomRight)
            anchor = LPositioner::AnchorTopRight;
    }
}

static void invertGravity(LPositioner::Gravity &gravity, bool x, bool y) noexcept
{
    if (x)
    {
        if (gravity == LPositioner::GravityLeft)
            gravity = LPositioner::GravityRight;
        else if (gravity == LPositioner::GravityRight)
            gravity = LPositioner::GravityLeft;
        else if (gravity == LPositioner::GravityTopLeft)
            gravity = LPositioner::GravityTopRight;
        else if (gravity == LPositioner::GravityTopRight)
            gravity = LPositioner::GravityTopLeft;
        else if (gravity == LPositioner::GravityBottomLeft)
            gravity = LPositioner::GravityBottomRight;
        else if (gravity == LPositioner::GravityBottomRight)
            gravity = LPositioner::GravityBottomLeft;
    }

    if (y)
    {
        if (gravity == LPositioner::GravityTop)
            gravity = LPositioner::GravityBottom;
        else if (gravity == LPositioner::GravityBottom)
            gravity = LPositioner::GravityTop;
        else if (gravity == LPositioner::GravityTopLeft)
            gravity = LPositioner::GravityBottomLeft;
        else if (gravity == LPositioner::GravityBottomLeft)
            gravity = LPositioner::GravityTopLeft;
        else if (gravity == LPositioner::GravityTopRight)
            gravity = LPositioner::GravityBottomRight;
        else if (gravity == LPositioner::GravityBottomRight)
            gravity = LPositioner::GravityTopRight;
    }
}

static void updateAnchorOrigin(Config &config) noexcept
{
    switch(config.anchor) {
    case LPositioner::Anchor::NoAnchor:
        config.anchorOrigin.fX = config.popup->positioner().anchorRect().width()/2;
        config.anchorOrigin.fY = config.popup->positioner().anchorRect().height()/2;
        break;
    case LPositioner::AnchorTop:
        config.anchorOrigin.fX = config.popup->positioner().anchorRect().width()/2;
        config.anchorOrigin.fY = 0;
        break;
    case LPositioner::AnchorBottom:
        config.anchorOrigin.fX = config.popup->positioner().anchorRect().width()/2;
        config.anchorOrigin.fY = config.popup->positioner().anchorRect().height();
        break;
    case LPositioner::AnchorLeft:
        config.anchorOrigin.fX = 0;
        config.anchorOrigin.fY = config.popup->positioner().anchorRect().height()/2;
        break;
    case LPositioner::AnchorRight:
        config.anchorOrigin.fX = config.popup->positioner().anchorRect().width();
        config.anchorOrigin.fY = config.popup->positioner().anchorRect().height()/2;
        break;
    case LPositioner::AnchorTopLeft:
        config.anchorOrigin.fX = 0;
        config.anchorOrigin.fY = 0;
        break;
    case LPositioner::AnchorBottomLeft:
        config.anchorOrigin.fX = 0;
        config.anchorOrigin.fY = config.popup->positioner().anchorRect().height();
        break;
    case LPositioner::AnchorTopRight:
        config.anchorOrigin.fX = config.popup->positioner().anchorRect().width();
        config.anchorOrigin.fY = 0;
        break;
    case LPositioner::AnchorBottomRight:
        config.anchorOrigin.fX = config.popup->positioner().anchorRect().width();
        config.anchorOrigin.fY = config.popup->positioner().anchorRect().height();
        break;
    }
}

static void updatePopupOrigin(Config &config) noexcept
{
    switch(config.gravity) {
    case LPositioner::Gravity::NoGravity:
        config.popupOrigin.fX = config.finalRect.width()/2;
        config.popupOrigin.fY = config.finalRect.height()/2;
        break;
    case LPositioner::GravityTop:
        config.popupOrigin.fX = config.finalRect.width()/2;
        config.popupOrigin.fY = config.finalRect.height();
        break;
    case LPositioner::GravityBottom:
        config.popupOrigin.fX = config.finalRect.width()/2;
        config.popupOrigin.fY = 0;
        break;
    case LPositioner::GravityLeft:
        config.popupOrigin.fX = config.finalRect.width();
        config.popupOrigin.fY = config.finalRect.height()/2;
        break;
    case LPositioner::GravityRight:
        config.popupOrigin.fX = 0;
        config.popupOrigin.fY = config.finalRect.height()/2;
        break;
    case LPositioner::GravityTopLeft:
        config.popupOrigin.fX = config.finalRect.width();
        config.popupOrigin.fY = config.finalRect.height();
        break;
    case LPositioner::GravityBottomLeft:
        config.popupOrigin.fX = config.finalRect.width();
        config.popupOrigin.fY = 0;
        break;
    case LPositioner::GravityTopRight:
        config.popupOrigin.fX = 0;
        config.popupOrigin.fY = config.finalRect.height();
        break;
    case LPositioner::GravityBottomRight:
        config.popupOrigin.fX = 0;
        config.popupOrigin.fY = 0;
        break;
    }
}

static void updatefinalRectPos(Config &config) noexcept
{
    const SkIPoint finalPos =
        config.parentPos +
        config.anchorOrigin -
        config.popupOrigin +
        config.slide +
        config.popup->positioner().anchorRect().topLeft() +
        config.popup->positioner().offset();

    config.finalRect.offsetTo(finalPos.x(), finalPos.y());
}

CZBitset<LEdge> LPopupRole::constrainedEdges(const SkIRect &rect) const noexcept
{
    CZBitset<LEdge> edges;

    if (bounds().width() > 0)
    {
        if (rect.x() < bounds().x())
            edges.add(LEdgeLeft);

        if (rect.x() + rect.width() > bounds().x() + bounds().width())
            edges.add(LEdgeRight);
    }

    if (bounds().height() > 0)
    {
        if (rect.y() < bounds().y())
            edges.add(LEdgeTop);

        if (rect.y() + rect.height() > bounds().y() + bounds().height())
            edges.add(LEdgeBottom);
    }

    return edges;
}

static void updateConfig(Config &conf) noexcept
{
    updateAnchorOrigin(conf);
    updatePopupOrigin(conf);
    updatefinalRectPos(conf);
}

SkIRect LPopupRole::calculateUnconstrainedRect(const SkIPoint *futureParentPos) const noexcept
{
    Config conf;
    conf.popup = this;
    conf.anchor = positioner().anchor();
    conf.gravity = positioner().gravity();
    conf.finalRect.setXYWH(
        conf.finalRect.x(),
        conf.finalRect.y(),
        positioner().size().width(),
        positioner().size().height());

    if (futureParentPos)
        conf.parentPos = *futureParentPos;
    else
    {
        conf.parentPos = surface()->parent()->rolePos();

        if (surface()->parent()->toplevel())
            conf.parentPos += surface()->parent()->toplevel()->windowGeometry().topLeft();
        else if (surface()->parent()->popup())
            conf.parentPos += surface()->parent()->popup()->windowGeometry().topLeft();
    }

    updateConfig(conf);

    CZBitset<LEdge> constrained { constrainedEdges(conf.finalRect) };

    if (constrained == 0)
        goto final;

    // Handle X axis
    if (constrained.has(LEdgeLeft | LEdgeRight))
    {
        if (positioner().constraintAdjustments().has(LPositioner::FlipX))
        {
            invertAnchor(conf.anchor, true, false);
            invertGravity(conf.gravity, true, false);
            updateConfig(conf);
            constrained = constrainedEdges(conf.finalRect);

            if (constrained == 0)
                goto final;
            else if (!constrained.has(LEdgeLeft | LEdgeRight))
                goto handleYAxis;
            else
            {
                // Revert
                invertAnchor(conf.anchor, true, false);
                invertGravity(conf.gravity, true, false);
                updateConfig(conf);
                constrained = constrainedEdges(conf.finalRect);
            }
        }

        if (positioner().constraintAdjustments().has(LPositioner::SlideX) && !constrained.hasAll(LEdgeLeft | LEdgeRight))
        {
            if (constrained.has(LEdgeLeft))
                conf.slide.fX = bounds().x() - conf.finalRect.x();
            else
                conf.slide.fX = bounds().x() + bounds().width() - conf.finalRect.x() - conf.finalRect.width();

            updateConfig(conf);
            constrained = constrainedEdges(conf.finalRect);

            if (constrained == 0)
                goto final;
            else if (!constrained.has(LEdgeLeft | LEdgeRight))
                goto handleYAxis;
            else
            {
                // Revert
                conf.slide.fX = 0;
                updateConfig(conf);
                constrained = constrainedEdges(conf.finalRect);
            }
        }

        if (positioner().constraintAdjustments().has(LPositioner::ResizeX))
        {
            if (positioner().constraintAdjustments().has(LPositioner::SlideX))
            {
                conf.finalRect.fRight = conf.finalRect.fLeft + bounds().width();
                conf.slide.fX = bounds().x() - conf.finalRect.x();
            }
            else if (constrained.has(LEdgeRight))
            {
                conf.finalRect.fRight = bounds().x() + bounds().width();
            }

            updateConfig(conf);
        }
    }

    handleYAxis:

    if (constrained.has(LEdgeTop | LEdgeBottom))
    {
        if (positioner().constraintAdjustments().has(LPositioner::FlipY))
        {
            invertAnchor(conf.anchor, false, true);
            invertGravity(conf.gravity, false, true);
            updateConfig(conf);
            constrained = constrainedEdges(conf.finalRect);

            if (!constrained.has(LEdgeTop | LEdgeBottom))
                goto final;
            else
            {
                // Revert
                invertAnchor(conf.anchor, false, true);
                invertGravity(conf.gravity, false, true);
                updateConfig(conf);
                constrained = constrainedEdges(conf.finalRect);
            }
        }

        if (positioner().constraintAdjustments().has(LPositioner::SlideY) && !constrained.hasAll(LEdgeTop | LEdgeBottom))
        {
            if (constrained.has(LEdgeTop))
                conf.slide.fY = bounds().y() - conf.finalRect.y();
            else // Bottom
                conf.slide.fY = bounds().y() + bounds().height() - conf.finalRect.y() - conf.finalRect.height();

            updateConfig(conf);
            constrained = constrainedEdges(conf.finalRect);

            if (!constrained.has(LEdgeTop | LEdgeBottom))
                goto final;
            else
            {
                // Revert
                conf.slide.fY = 0;
                updateConfig(conf);
                constrained = constrainedEdges(conf.finalRect);
            }
        }

        if (positioner().constraintAdjustments().has(LPositioner::ResizeY))
        {
            if (positioner().constraintAdjustments().has(LPositioner::SlideY))
            {
                conf.finalRect.fBottom = conf.finalRect.fTop + bounds().height();
                conf.slide.fY = bounds().y() - conf.finalRect.y();
            }
            else if (constrained.has(LEdgeBottom))
                conf.finalRect.fBottom = bounds().y() + bounds().height();

            updateConfig(conf);
        }
    }


    final:
    conf.finalRect.offset(-conf.parentPos);
    return conf.finalRect;
}

LPopupRole::LPopupRole(const void *params) noexcept :
    LBaseSurfaceRole(FactoryObjectType,
        static_cast<const LPopupRole::Params*>(params)->popup,
        static_cast<const LPopupRole::Params*>(params)->surface,
        LSurface::Role::Popup),
    m_positioner(*static_cast<const LPopupRole::Params*>(params)->positioner)
{}

LPopupRole::~LPopupRole()
{
    validateDestructor();
    notifyDestruction();
}

const LPopupRole::Configuration *LPopupRole::findConfiguration(UInt32 serial) const noexcept
{
    for (auto &conf : m_sentConfs)
        if (conf.serial == serial)
            return &conf;

    if (m_pendingConfiguration.serial == serial)
        return &m_pendingConfiguration;
    if (m_lastACKConfiguration.serial == serial)
        return &m_lastACKConfiguration;

    return nullptr;
}

bool LPopupRole::isTopmostPopup() const noexcept
{
    std::list<LSurface*>::const_reverse_iterator s = compositor()->surfaces().rbegin();
    for (; s!= compositor()->surfaces().rend(); s++)
        if ((*s)->popup() && (*s)->client() == surface()->client())
                return (*s)->popup() == this;

    return false;
}

void LPopupRole::setExclusiveOutput(LOutput *output) noexcept
{
    m_exclusiveOutput.reset(output);
}

void LPopupRole::configureRect(const SkIRect &rect) const noexcept
{
    auto &res { *static_cast<XdgShell::RXdgPopup*>(resource()) };

    if (!res.xdgSurfaceRes())
        return;

    if (!m_flags.has(CanBeConfigured))
        return;

    if (!m_flags.has(HasConfigurationToSend))
    {
        m_flags.add(HasConfigurationToSend);
        m_pendingConfiguration.serial = LTime::nextSerial();
    }

    m_pendingConfiguration.rect.setXYWH(
        rect.x(),
        rect.y(),
        rect.width() < 0 ? 0 : rect.width(),
        rect.height() < 0 ? 0 : rect.height());
}

void LPopupRole::dismiss()
{
    std::list<LSurface*>::const_reverse_iterator s = compositor()->surfaces().rbegin();
    for (; s!= compositor()->surfaces().rend(); s++)
    {
        if ((*s)->popup() && ((*s)->isSubchildOf(surface()) || *s == surface() ))
        {
            if (!(*s)->popup()->m_flags.has(Dismissed))
            {
                XdgShell::RXdgPopup *res = (XdgShell::RXdgPopup*)resource();
                res->popupDone();
                (*s)->popup()->m_flags.add(Dismissed);
                (*s)->imp()->setKeyboardGrabToParent();
                (*s)->imp()->setMapped(false);
            }

            if ((*s) == surface())
                return;
        }
    }
}

void LPopupRole::handleSurfaceCommit(CommitOrigin origin)
{
    L_UNUSED(origin);

    if (m_flags.has(Dismissed))
        return;

    if (surface()->imp()->pendingParent)
    {
        if (surface()->imp()->isInChildrenOrPendingChildren(surface()->imp()->pendingParent))
        {
            xdgSurfaceResource()->postError(
                XDG_WM_BASE_ERROR_INVALID_POPUP_PARENT,
                "Parent can not be child or equal to surface.");
            return;
        }

        surface()->imp()->pendingParent->imp()->applyPendingChildren();
    }

    if (!surface()->parent())
    {
        xdgSurfaceResource()->postError(
            XDG_WM_BASE_ERROR_INVALID_POPUP_PARENT,
            "Popup has no parent.");
        return;
    }

    // Configure request
    if (m_flags.has(HasPendingInitialConf))
    {
        if (surface()->hasBuffer())
        {
            resource()->postError(XDG_SURFACE_ERROR_UNCONFIGURED_BUFFER, "Attaching a buffer to an unconfigured surface");
            return;
        }

        surface()->imp()->setLayer(LLayerOverlay);

        m_flags.remove(HasPendingInitialConf);
        m_flags.add(CanBeConfigured);
        configureRequest();

        if (!m_flags.has(HasConfigurationToSend))
            configureRect(calculateUnconstrainedRect());

        return;
    }

    // Request unmap
    if (surface()->mapped() && (!surface()->hasBuffer() || !surface()->parent()))
    {
        dismiss();
        return;
    }

    fullAtomsUpdate();

    // Request map
    if (!surface()->mapped() && surface()->hasBuffer() && surface()->parent() && surface()->parent()->mapped())
        surface()->imp()->setMapped(true);
}

void LPopupRole::fullAtomsUpdate()
{
    if (xdgSurfaceResource()->m_hasPendingWindowGeometry)
    {
        xdgSurfaceResource()->m_hasPendingWindowGeometry = false;
        xdgSurfaceResource()->m_currentWindowGeometry = xdgSurfaceResource()->m_pendingWindowGeometry;
    }
    else if (!xdgSurfaceResource()->m_windowGeometrySet)
    {
        const SkIRect newGeometry { xdgSurfaceResource()->calculateGeometryWithSubsurfaces() };

        if (newGeometry != xdgSurfaceResource()->m_currentWindowGeometry)
            xdgSurfaceResource()->m_currentWindowGeometry = newGeometry;
    }

    pendingAtoms().localPos = m_lastACKConfiguration.rect.topLeft();
    pendingAtoms().serial = m_lastACKConfiguration.serial;
    pendingAtoms().windowGeometry = xdgSurfaceResource()->m_currentWindowGeometry;

    CZBitset<AtomChanges> changesToNotify;

    if (currentAtoms().windowGeometry != pendingAtoms().windowGeometry)
        changesToNotify.add(WindowGeometryChanged);

    if (currentAtoms().localPos != pendingAtoms().localPos)
        changesToNotify.add(LocalPosChanged);

    if (currentAtoms().serial != pendingAtoms().serial)
        changesToNotify.add(SerialChanged);

    if (changesToNotify != 0)
    {
        m_currentAtomsIndex = 1 - m_currentAtomsIndex;
        atomsChanged(0, pendingAtoms());
        pendingAtoms() = currentAtoms();
    }
}

void LPopupRole::sendPendingConfiguration() noexcept
{
    if (!m_flags.has(HasConfigurationToSend))
        return;

    m_flags.remove(HasConfigurationToSend);

    if (m_flags.has(HasPendingReposition))
    {
        m_flags.remove(HasPendingReposition);
        xdgPopupResource()->repositioned(m_repositionToken);
    }

    xdgPopupResource()->configure(m_pendingConfiguration.rect);
    xdgSurfaceResource()->configure(m_pendingConfiguration.serial);
    m_sentConfs.emplace_back(m_pendingConfiguration);
}

XdgShell::RXdgPopup *LPopupRole::xdgPopupResource() const noexcept
{
    return static_cast<XdgShell::RXdgPopup*>(resource());
}

XdgShell::RXdgSurface *LPopupRole::xdgSurfaceResource() const noexcept
{
    return xdgPopupResource()->xdgSurfaceRes();
}
