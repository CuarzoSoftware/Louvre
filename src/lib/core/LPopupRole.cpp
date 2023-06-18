#include <protocols/XdgShell/private/RXdgSurfacePrivate.h>
#include <protocols/XdgShell/RXdgPopup.h>
#include <protocols/XdgShell/xdg-shell.h>
#include <private/LPopupRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LBaseSurfaceRolePrivate.h>
#include <private/LPositionerPrivate.h>
#include <private/LPointerPrivate.h>
#include <LRect.h>
#include <LCompositor.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LKeyboard.h>

using namespace Louvre;

LPopupRole::LPopupRole(LPopupRole::Params *params) : LBaseSurfaceRole(params->popup, params->surface, LSurface::Role::Popup)
{
   m_imp = new LPopupRolePrivate();
   imp()->positioner.imp()->data = params->positioner->imp()->data;
   imp()->positioner.imp()->updateGlobalScale();
}

LPopupRole::~LPopupRole()
{
    if (surface())
        surface()->imp()->setMapped(false);

    compositor()->destroyPopupRoleRequest(this);

    delete m_imp;
}

void LPopupRole::configureC(const LRect &r)
{
    XdgShell::RXdgPopup *res = (XdgShell::RXdgPopup*)resource();

    if (!res->rXdgSurface())
        return;

    LRect rect = r/compositor()->globalScale();
    res->configure(rect.x(), rect.y(), rect.w(), rect.h());
    res->rXdgSurface()->configure(LCompositor::nextSerial());
}

void LPopupRole::sendPopupDoneEvent()
{
    if (imp()->dismissed)
        return;

    list<LSurface*>::const_reverse_iterator s = surface()->children().rbegin();
    for (; s!= surface()->children().rend(); s++)
    {
        if ((*s)->popup())
            (*s)->popup()->sendPopupDoneEvent();
    }

    XdgShell::RXdgPopup *res = (XdgShell::RXdgPopup*)resource();

    res->popup_done();
    surface()->imp()->mapped = false;
    surface()->mappingChanged();
    imp()->dismissed = true;
}

void LPopupRole::sendRepositionedEvent(UInt32 token)
{
    XdgShell::RXdgPopup *res = (XdgShell::RXdgPopup*)resource();
    res->repositioned(token);
}

const LRect &LPopupRole::windowGeometryS() const
{
    return xdgSurfaceResource()->imp()->currentWindowGeometryS;
}

const LRect &LPopupRole::windowGeometryC() const
{
    return xdgSurfaceResource()->imp()->currentWindowGeometryC;
}

const LPositioner &LPopupRole::positioner() const
{
    return imp()->positioner;
}

void LPopupRole::handleSurfaceCommit(Protocols::Wayland::RSurface::CommitOrigin origin)
{
    // Commit inicial para asignar rol y padre
    if (surface()->imp()->pending.role)
    {
        // Based on xdg_surface documentation
        if (surface()->imp()->hasBufferOrPendingBuffer())
        {
            wl_resource_post_error(surface()->surfaceResource()->resource(),
                                   0,
                                   "wl_surface attach before first xdg_surface configure");
            return;
        }

        if (surface()->imp()->pendingParent)
            surface()->imp()->pendingParent->imp()->applyPendingChildren();

        surface()->imp()->applyPendingRole();
        surface()->popup()->configureRequest();
        return;
    }

    // Cambios double-buffered
    if (xdgSurfaceResource()->imp()->hasPendingWindowGeometry)
    {
        xdgSurfaceResource()->imp()->hasPendingWindowGeometry = false;
        xdgSurfaceResource()->imp()->currentWindowGeometryS = xdgSurfaceResource()->imp()->pendingWindowGeometryS;
        xdgSurfaceResource()->imp()->currentWindowGeometryC = xdgSurfaceResource()->imp()->pendingWindowGeometryS * compositor()->globalScale();
        geometryChanged();
    }
    // Si nunca ha asignado la geometría, usa el tamaño de la superficie
    else if (!xdgSurfaceResource()->imp()->windowGeometrySet)
    {
        xdgSurfaceResource()->imp()->currentWindowGeometryS = LRect(0, surface()->sizeS());
        xdgSurfaceResource()->imp()->currentWindowGeometryC = LRect(0, surface()->sizeC());
        geometryChanged();
    }

    // Solicita volver a mapear
    if (!surface()->mapped() && !surface()->buffer())
    {
        configureRequest();
        return;
    }

    // Solicita desmapear
    if (surface()->mapped() && !surface()->buffer())
    {
        surface()->imp()->setMapped(false);
        surface()->imp()->setParent(nullptr);
        return;
    }

    // Si no estaba mapeada y añade buffer la mappeamos
    if (!surface()->mapped() && surface()->buffer())
        surface()->imp()->setMapped(true);
}

void LPopupRole::globalScaleChanged(Int32 oldScale, Int32 newScale)
{
    L_UNUSED(oldScale);

    // Window geometry
    xdgSurfaceResource()->imp()->currentWindowGeometryC = xdgSurfaceResource()->imp()->currentWindowGeometryS * newScale;

    // Positioner bounds
    imp()->positionerBoundsC = imp()->positionerBoundsS * newScale;

    // Positioner
    positioner().imp()->updateGlobalScale();
}

void LPopupRole::setPositionerBoundsC(const LRect &bounds)
{
    imp()->positionerBoundsC = bounds;
    imp()->positionerBoundsS = bounds / compositor()->globalScale();
}

const LRect &LPopupRole::positionerBoundsC() const
{
    return imp()->positionerBoundsC;
}

XdgShell::RXdgPopup *LPopupRole::xdgPopupResource() const
{
    return (XdgShell::RXdgPopup*)resource();
}

XdgShell::RXdgSurface *LPopupRole::xdgSurfaceResource() const
{
    return xdgPopupResource()->rXdgSurface();
}

const LRect &LPopupRole::positionerBoundsS() const
{
    return imp()->positionerBoundsS;
}
