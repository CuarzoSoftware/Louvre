#include <protocols/XdgShell/RXdgSurface.h>
#include <protocols/XdgShell/RXdgPopup.h>
#include <protocols/XdgShell/xdg-shell.h>
#include <private/LPopupRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LPointerPrivate.h>
#include <LPositioner.h>
#include <LCompositor.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LTime.h>
#include <LRect.h>

using namespace Louvre;

struct Config
{
    const LPopupRole *popup;
    LRect finalRect;
    LPoint parentPos;
    LPositioner::Gravity gravity;
    LPositioner::Anchor anchor;
    LPoint anchorOrigin;
    LPoint popupOrigin;
    LPoint slide;
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
        config.anchorOrigin = config.popup->positioner().anchorRect().size()/2;
        break;
    case LPositioner::AnchorTop:
        config.anchorOrigin.setX(config.popup->positioner().anchorRect().w()/2);
        config.anchorOrigin.setY(0);
        break;
    case LPositioner::AnchorBottom:
        config.anchorOrigin.setX(config.popup->positioner().anchorRect().w()/2);
        config.anchorOrigin.setY(config.popup->positioner().anchorRect().h());
        break;
    case LPositioner::AnchorLeft:
        config.anchorOrigin.setX(0);
        config.anchorOrigin.setY(config.popup->positioner().anchorRect().h()/2);
        break;
    case LPositioner::AnchorRight:
        config.anchorOrigin.setX(config.popup->positioner().anchorRect().w());
        config.anchorOrigin.setY(config.popup->positioner().anchorRect().h()/2);
        break;
    case LPositioner::AnchorTopLeft:
        config.anchorOrigin.setX(0);
        config.anchorOrigin.setY(0);
        break;
    case LPositioner::AnchorBottomLeft:
        config.anchorOrigin.setX(0);
        config.anchorOrigin.setY(config.popup->positioner().anchorRect().h());
        break;
    case LPositioner::AnchorTopRight:
        config.anchorOrigin.setX(config.popup->positioner().anchorRect().w());
        config.anchorOrigin.setY(0);
        break;
    case LPositioner::AnchorBottomRight:
        config.anchorOrigin = config.popup->positioner().anchorRect().size();
        break;
    }
}

static void updatePopupOrigin(Config &config) noexcept
{
    switch(config.gravity) {
    case LPositioner::Gravity::NoGravity:
        config.popupOrigin = config.finalRect.size()/2;
        break;
    case LPositioner::GravityTop:
        config.popupOrigin.setX(config.finalRect.w()/2);
        config.popupOrigin.setY(config.finalRect.h());
        break;
    case LPositioner::GravityBottom:
        config.popupOrigin.setX(config.finalRect.w()/2);
        config.popupOrigin.setY(0);
        break;
    case LPositioner::GravityLeft:
        config.popupOrigin.setX(config.finalRect.w());
        config.popupOrigin.setY(config.finalRect.h()/2);
        break;
    case LPositioner::GravityRight:
        config.popupOrigin.setX(0);
        config.popupOrigin.setY(config.finalRect.h()/2);
        break;
    case LPositioner::GravityTopLeft:
        config.popupOrigin = config.finalRect.size();
        break;
    case LPositioner::GravityBottomLeft:
        config.popupOrigin.setX(config.finalRect.w());
        config.popupOrigin.setY(0);
        break;
    case LPositioner::GravityTopRight:
        config.popupOrigin.setX(0);
        config.popupOrigin.setY(config.finalRect.h());
        break;
    case LPositioner::GravityBottomRight:
        config.popupOrigin.setX(0);
        config.popupOrigin.setY(0);
        break;
    }
}

static void updateFinalRectPos(Config &config) noexcept
{
    config.finalRect.setPos(
        config.parentPos +
        config.anchorOrigin -
        config.popupOrigin +
        config.slide +
        config.popup->positioner().anchorRect().pos() +
        config.popup->positioner().offset());
}

LBitset<LEdge> LPopupRole::constrainedEdges(const LRect &rect) const noexcept
{
    LBitset<LEdge> edges;

    if (bounds().w() > 0)
    {
        if (rect.x() < bounds().x())
            edges.add(LEdgeLeft);

        if (rect.x() + rect.w() > bounds().x() + bounds().w())
            edges.add(LEdgeRight);
    }

    if (bounds().h() > 0)
    {
        if (rect.y() < bounds().y())
            edges.add(LEdgeTop);

        if (rect.y() + rect.h() > bounds().y() + bounds().h())
            edges.add(LEdgeBottom);
    }

    return edges;
}

static void updateConfig(Config &conf) noexcept
{
    updateAnchorOrigin(conf);
    updatePopupOrigin(conf);
    updateFinalRectPos(conf);
}

LRect LPopupRole::calculateUnconstrainedRect(const LPoint *futureParentPos) const noexcept
{
    Config conf;
    conf.popup = this;
    conf.anchor = positioner().anchor();
    conf.gravity = positioner().gravity();
    conf.finalRect.setSize(positioner().size());

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

    LBitset<LEdge> constrained { constrainedEdges(conf.finalRect) };

    if (constrained == 0)
        goto final;

    // Handle X axis
    if (constrained.check(LEdgeLeft | LEdgeRight))
    {
        if (positioner().constraintAdjustments().check(LPositioner::FlipX))
        {
            invertAnchor(conf.anchor, true, false);
            invertGravity(conf.gravity, true, false);
            updateConfig(conf);
            constrained = constrainedEdges(conf.finalRect);

            if (constrained == 0)
                goto final;
            else if (!constrained.check(LEdgeLeft | LEdgeRight))
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

        if (positioner().constraintAdjustments().check(LPositioner::SlideX) && !constrained.checkAll(LEdgeLeft | LEdgeRight))
        {
            if (constrained.check(LEdgeLeft))
                conf.slide.setX(bounds().x() - conf.finalRect.x());
            else
                conf.slide.setX(bounds().x() + bounds().w() - conf.finalRect.x() - conf.finalRect.w());

            updateConfig(conf);
            constrained = constrainedEdges(conf.finalRect);

            if (constrained == 0)
                goto final;
            else if (!constrained.check(LEdgeLeft | LEdgeRight))
                goto handleYAxis;
            else
            {
                // Revert
                conf.slide.setX(0);
                updateConfig(conf);
                constrained = constrainedEdges(conf.finalRect);
            }
        }

        if (positioner().constraintAdjustments().check(LPositioner::ResizeX))
        {
            if (positioner().constraintAdjustments().check(LPositioner::SlideX))
            {
                conf.finalRect.setW(bounds().w());
                conf.slide.setX(bounds().x() - conf.finalRect.x());
            }
            else if (constrained.check(LEdgeRight))
                conf.finalRect.setW(bounds().x() + bounds().w() - conf.finalRect.x());

            updateConfig(conf);
        }
    }

    handleYAxis:

    if (constrained.check(LEdgeTop | LEdgeBottom))
    {
        if (positioner().constraintAdjustments().check(LPositioner::FlipY))
        {
            invertAnchor(conf.anchor, false, true);
            invertGravity(conf.gravity, false, true);
            updateConfig(conf);
            constrained = constrainedEdges(conf.finalRect);

            if (!constrained.check(LEdgeTop | LEdgeBottom))
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

        if (positioner().constraintAdjustments().check(LPositioner::SlideY) && !constrained.checkAll(LEdgeTop | LEdgeBottom))
        {
            if (constrained.check(LEdgeTop))
                conf.slide.setY(bounds().y() - conf.finalRect.y());
            else // Bottom
                conf.slide.setY(bounds().y() + bounds().h() - conf.finalRect.y() - conf.finalRect.h());

            updateConfig(conf);
            constrained = constrainedEdges(conf.finalRect);

            if (!constrained.check(LEdgeTop | LEdgeBottom))
                goto final;
            else
            {
                // Revert
                conf.slide.setY(0);
                updateConfig(conf);
                constrained = constrainedEdges(conf.finalRect);
            }
        }

        if (positioner().constraintAdjustments().check(LPositioner::ResizeY))
        {
            if (positioner().constraintAdjustments().check(LPositioner::SlideY))
            {
                conf.finalRect.setH(bounds().h());
                conf.slide.setY(bounds().y() - conf.finalRect.y());
            }
            else if (constrained.check(LEdgeBottom))
                conf.finalRect.setH(bounds().y() + bounds().h() - conf.finalRect.y());

            updateConfig(conf);
        }
    }


    final:
    conf.finalRect.setPos(conf.finalRect.pos() - conf.parentPos);
    return conf.finalRect;
}

LPopupRole::LPopupRole(const void *params) noexcept :
    LBaseSurfaceRole(FactoryObjectType,
        static_cast<const LPopupRole::Params*>(params)->popup,
        static_cast<const LPopupRole::Params*>(params)->surface,
        LSurface::Role::Popup),
    m_positioner(*static_cast<const LPopupRole::Params*>(params)->positioner)
{}

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

void LPopupRole::configureRect(const LRect &rect) const noexcept
{
    auto &res { *static_cast<XdgShell::RXdgPopup*>(resource()) };

    if (!res.xdgSurfaceRes())
        return;

    if (!m_flags.check(CanBeConfigured))
        return;

    if (!m_flags.check(HasConfigurationToSend))
    {
        m_flags.add(HasConfigurationToSend);
        m_pendingConfiguration.serial = LTime::nextSerial();
    }

    m_pendingConfiguration.rect.setPos(rect.pos());
    m_pendingConfiguration.rect.setW(rect.w() < 0 ? 0 : rect.w());
    m_pendingConfiguration.rect.setH(rect.h() < 0 ? 0 : rect.h());
}

void LPopupRole::dismiss()
{
    std::list<LSurface*>::const_reverse_iterator s = compositor()->surfaces().rbegin();
    for (; s!= compositor()->surfaces().rend(); s++)
    {
        if ((*s)->popup() && ((*s)->isSubchildOf(surface()) || *s == surface() ))
        {
            if (!(*s)->popup()->m_flags.check(Dismissed))
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

    if (m_flags.check(Dismissed))
        return;

    if (surface()->imp()->pendingParent)
    {
        if (surface()->imp()->isInChildrenOrPendingChildren(surface()->imp()->pendingParent))
        {
            wl_resource_post_error(xdgSurfaceResource()->resource(),
                                   XDG_WM_BASE_ERROR_INVALID_POPUP_PARENT,
                                   "Parent can not be child or equal to surface.");
            return;
        }

        surface()->imp()->pendingParent->imp()->applyPendingChildren();
    }

    if (!surface()->parent())
    {
        wl_resource_post_error(xdgSurfaceResource()->resource(),
                               XDG_WM_BASE_ERROR_INVALID_POPUP_PARENT,
                               "Popup has no parent.");
        return;
    }

    // Configure request
    if (m_flags.check(HasPendingInitialConf))
    {
        if (surface()->hasBuffer())
        {
            wl_resource_post_error(resource()->resource(), XDG_SURFACE_ERROR_UNCONFIGURED_BUFFER, "Attaching a buffer to an unconfigured surface");
            return;
        }

        surface()->imp()->setLayer(LLayerOverlay);

        m_flags.remove(HasPendingInitialConf);
        m_flags.add(CanBeConfigured);
        configureRequest();

        if (!m_flags.check(HasConfigurationToSend))
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
        const LRect newGeometry { xdgSurfaceResource()->calculateGeometryWithSubsurfaces() };

        if (newGeometry != xdgSurfaceResource()->m_currentWindowGeometry)
            xdgSurfaceResource()->m_currentWindowGeometry = newGeometry;
    }

    pendingAtoms().localPos = m_lastACKConfiguration.rect.pos();
    pendingAtoms().serial = m_lastACKConfiguration.serial;
    pendingAtoms().windowGeometry = xdgSurfaceResource()->m_currentWindowGeometry;

    LBitset<AtomChanges> changesToNotify;

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
    if (!m_flags.check(HasConfigurationToSend))
        return;

    m_flags.remove(HasConfigurationToSend);

    if (m_flags.check(HasPendingReposition))
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
