#include <protocols/XdgShell/RXdgPopup.h>
#include <protocols/XdgShell/RXdgSurface.h>

#include <private/LPopupRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LBaseSurfaceRolePrivate.h>
#include <private/LPositionerPrivate.h>

#include <protocols/XdgShell/xdg-shell.h>

#include <LRect.h>
#include <LWayland.h>
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
    res->rXdgSurface()->configure(LWayland::nextSerial());
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
    return imp()->currentWindowGeometryS;
}

const LRect &LPopupRole::windowGeometryC() const
{
    return imp()->currentWindowGeometryC;
}

const LPositioner &LPopupRole::positioner() const
{
    return imp()->positioner;
}

void LPopupRole::handleSurfaceCommit()
{
    // Commit inicial para asignar rol y padre
    if (surface()->imp()->pending.role)
    {

        if (surface()->buffer())
        {
            wl_resource_post_error(resource()->resource(), XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED, "Given wl_surface already has a buffer attached.");
            return;
        }

        if (surface()->imp()->pendingParent)
        {

            surface()->imp()->setParent(surface()->imp()->pendingParent);
            surface()->imp()->pendingParent = nullptr;
        }
        else
        {
            wl_resource_post_error(resource()->resource(), XDG_WM_BASE_ERROR_INVALID_POPUP_PARENT, "xdg_surface.get_popup with invalid popup parent.");
            return;
        }

        surface()->imp()->applyPendingRole();
        surface()->popup()->configureRequest();
        return;
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

    // Cambios double-buffered
    if (imp()->hasPendingWindowGeometry)
    {
        imp()->hasPendingWindowGeometry = false;
        imp()->currentWindowGeometryS = imp()->pendingWindowGeometryS;
        imp()->currentWindowGeometryC = imp()->pendingWindowGeometryS * compositor()->globalScale();
        geometryChanged();
    }
    // Si nunca ha asignado la geometría, usa el tamaño de la superficie
    else if (!imp()->windowGeometrySet && imp()->currentWindowGeometryC.size() != surface()->sizeC())
    {
        imp()->currentWindowGeometryS = LRect(0, surface()->sizeS());
        imp()->currentWindowGeometryC = LRect(0, surface()->sizeC());
        geometryChanged();
    }

    // Si no estaba mapeada y añade buffer la mappeamos
    if (!surface()->mapped() && surface()->buffer())
        surface()->imp()->setMapped(true);
}

void LPopupRole::globalScaleChanged(Int32 oldScale, Int32 newScale)
{
    L_UNUSED(oldScale);

    // Window geometry
    imp()->currentWindowGeometryC = imp()->currentWindowGeometryS * newScale;

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

const LRect &LPopupRole::positionerBoundsS() const
{
    return imp()->positionerBoundsS;
}
