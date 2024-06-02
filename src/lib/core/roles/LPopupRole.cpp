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

const LPopupRole::Configuration &LPopupRole::current() const noexcept
{
    return imp()->current;
}

const LPopupRole::Configuration &LPopupRole::pending() const noexcept
{
    return imp()->pending;
}

const LPopupRole::Configuration &LPopupRole::previous() const noexcept
{
    return imp()->previous;
}

LBitset<LPopupRole::ConstrainedEdges> LPopupRole::constrainedEdges(const LRect &rect) const noexcept
{
    LBitset<LPopupRole::ConstrainedEdges> edges;

    if (positionerBounds().w() > 0)
    {
        if (rect.x() < positionerBounds().x())
            edges.add(ConstrainedL);

        if (rect.x() + rect.w() > positionerBounds().x() + positionerBounds().w())
            edges.add(ConstrainedR);
    }

    if (positionerBounds().h() > 0)
    {
        if (rect.y() < positionerBounds().y())
            edges.add(ConstrainedT);

        if (rect.y() + rect.h() > positionerBounds().y() + positionerBounds().h())
            edges.add(ConstrainedB);
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

    LBitset<ConstrainedEdges> constrained { constrainedEdges(conf.finalRect) };

    if (constrained == 0)
        goto final;

    // Handle X axis
    if (constrained.check(ConstrainedL | ConstrainedR))
    {
        if (positioner().constraintAdjustments().check(LPositioner::FlipX))
        {
            invertAnchor(conf.anchor, true, false);
            invertGravity(conf.gravity, true, false);
            updateConfig(conf);
            constrained = constrainedEdges(conf.finalRect);

            if (constrained == 0)
                goto final;
            else if (!constrained.check(ConstrainedL | ConstrainedR))
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

        if (positioner().constraintAdjustments().check(LPositioner::SlideX) && !constrained.checkAll(ConstrainedL | ConstrainedR))
        {
            if (constrained.check(ConstrainedL))
                conf.slide.setX(positionerBounds().x() - conf.finalRect.x());
            else
                conf.slide.setX(positionerBounds().x() + positionerBounds().w() - conf.finalRect.x() - conf.finalRect.w());

            updateConfig(conf);
            constrained = constrainedEdges(conf.finalRect);

            if (constrained == 0)
                goto final;
            else if (!constrained.check(ConstrainedL | ConstrainedR))
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
                conf.finalRect.setW(positionerBounds().w());
                conf.slide.setX(positionerBounds().x() - conf.finalRect.x());
            }
            else if (constrained.check(ConstrainedR))
                conf.finalRect.setW(positionerBounds().x() + positionerBounds().w() - conf.finalRect.x());

            updateConfig(conf);
        }
    }

    handleYAxis:

    if (constrained.check(ConstrainedT | ConstrainedB))
    {
        if (positioner().constraintAdjustments().check(LPositioner::FlipY))
        {
            invertAnchor(conf.anchor, false, true);
            invertGravity(conf.gravity, false, true);
            updateConfig(conf);
            constrained = constrainedEdges(conf.finalRect);

            if (!constrained.check(ConstrainedT | ConstrainedB))
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

        if (positioner().constraintAdjustments().check(LPositioner::SlideY) && !constrained.checkAll(ConstrainedT | ConstrainedB))
        {
            if (constrained.check(ConstrainedT))
                conf.slide.setY(positionerBounds().y() - conf.finalRect.y());
            else // Bottom
                conf.slide.setY(positionerBounds().y() + positionerBounds().h() - conf.finalRect.y() - conf.finalRect.h());

            updateConfig(conf);
            constrained = constrainedEdges(conf.finalRect);

            if (!constrained.check(ConstrainedT | ConstrainedB))
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
                conf.finalRect.setH(positionerBounds().h());
                conf.slide.setY(positionerBounds().y() - conf.finalRect.y());
            }
            else if (constrained.check(ConstrainedB))
                conf.finalRect.setH(positionerBounds().y() + positionerBounds().h() - conf.finalRect.y());

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
    LPRIVATE_INIT_UNIQUE(LPopupRole)
{
    imp()->popup = this;
    imp()->positioner = *static_cast<const LPopupRole::Params*>(params)->positioner;
}

LPopupRole::~LPopupRole()
{
    // Required by pimpl
}

bool LPopupRole::isTopmostPopup() const
{
    if (!surface())
        return false;

    std::list<LSurface*>::const_reverse_iterator s = compositor()->surfaces().rbegin();
    for (; s!= compositor()->surfaces().rend(); s++)
        if ((*s)->popup() && (*s)->client() == surface()->client())
                return (*s)->popup() == this;

    return false;
}

void LPopupRole::configure(const LRect &rect) const
{
    auto &res { *static_cast<XdgShell::RXdgPopup*>(resource()) };

    if (!res.xdgSurfaceRes())
        return;

    if (!imp()->stateFlags.check(LPopupRolePrivate::CanBeConfigured))
        return;

    if (!positioner().reactive())
        imp()->stateFlags.remove(LPopupRolePrivate::CanBeConfigured);

    if (!imp()->stateFlags.check(LPopupRolePrivate::HasConfigurationToSend))
    {
        imp()->stateFlags.add(LPopupRolePrivate::HasConfigurationToSend);
        imp()->pending.serial = LTime::nextSerial();
    }

    imp()->pending.rect = rect;
}

void LPopupRole::dismiss()
{
    std::list<LSurface*>::const_reverse_iterator s = compositor()->surfaces().rbegin();
    for (; s!= compositor()->surfaces().rend(); s++)
    {
        if ((*s)->popup() && ((*s)->isSubchildOf(surface()) || *s == surface() ))
        {
            if (!imp()->stateFlags.check(LPopupRolePrivate::Dismissed))
            {
                XdgShell::RXdgPopup *res = (XdgShell::RXdgPopup*)resource();
                res->popupDone();
                imp()->stateFlags.add(LPopupRolePrivate::Dismissed);
            }

            if ((*s) == surface())
            {
                surface()->imp()->setMapped(false);
                return;
            }
        }
    }
}

const LRect &LPopupRole::windowGeometry() const
{
    return xdgSurfaceResource()->windowGeometry();
}

const LPositioner &LPopupRole::positioner() const
{
    return imp()->positioner;
}

void LPopupRole::handleSurfaceCommit(CommitOrigin origin)
{
    L_UNUSED(origin);

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

    if (surface()->imp()->pending.role)
    {
        if (surface()->imp()->hasBufferOrPendingBuffer())
        {
            wl_resource_post_error(surface()->surfaceResource()->resource(),
                                   XDG_SURFACE_ERROR_UNCONFIGURED_BUFFER,
                                   "Attaching a buffer to an unconfigured surface");
            return;
        }

        if (surface()->imp()->pendingParent)
            surface()->imp()->pendingParent->imp()->applyPendingChildren();

        surface()->imp()->applyPendingRole();
        surface()->imp()->setLayer(LLayerOverlay);
        imp()->stateFlags.add(LPopupRolePrivate::CanBeConfigured);
        configureRequest();
        if (!imp()->stateFlags.check(LPopupRolePrivate::HasConfigurationToSend))
            configure(calculateUnconstrainedRect());
        imp()->current.rect = imp()->pending.rect;
        return;
    }

    // Cambios double-buffered
    if (xdgSurfaceResource()->m_hasPendingWindowGeometry)
    {
        xdgSurfaceResource()->m_hasPendingWindowGeometry = false;
        xdgSurfaceResource()->m_currentWindowGeometry = xdgSurfaceResource()->m_pendingWindowGeometry;
        geometryChanged();
    }
    // Si nunca ha asignado la geometría, usa el tamaño de la superficie
    else if (!xdgSurfaceResource()->m_windowGeometrySet)
    {
        const LRect newGeometry { xdgSurfaceResource()->calculateGeometryWithSubsurfaces() };

        if (newGeometry != xdgSurfaceResource()->m_currentWindowGeometry)
        {
            xdgSurfaceResource()->m_currentWindowGeometry = newGeometry;
            geometryChanged();
        }
    }

    // Request configure
    if (!surface()->mapped() && !surface()->hasBuffer())
    {
        imp()->stateFlags.add(LPopupRolePrivate::CanBeConfigured);
        configureRequest();
        if (!imp()->stateFlags.check(LPopupRolePrivate::HasConfigurationToSend))
            configure(calculateUnconstrainedRect());
        imp()->current.rect = imp()->pending.rect;
        return;
    }

    // Request unmap
    if (surface()->mapped() && !surface()->hasBuffer())
    {
        surface()->imp()->setMapped(false);
        surface()->imp()->setKeyboardGrabToParent();
        surface()->imp()->setParent(nullptr);
        return;
    }

    // Request map
    if (!surface()->mapped() && surface()->hasBuffer() && surface()->parent() && surface()->parent()->mapped())
        surface()->imp()->setMapped(true);
}

void LPopupRole::setPositionerBounds(const LRect &bounds)
{
    imp()->positionerBounds = bounds;
}

const LRect &LPopupRole::positionerBounds() const
{
    return imp()->positionerBounds;
}

XdgShell::RXdgPopup *LPopupRole::xdgPopupResource() const
{
    return static_cast<XdgShell::RXdgPopup*>(resource());
}

XdgShell::RXdgSurface *LPopupRole::xdgSurfaceResource() const
{
    return xdgPopupResource()->xdgSurfaceRes();
}
