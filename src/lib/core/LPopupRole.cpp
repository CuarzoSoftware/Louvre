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


void LPopupRole::ping(UInt32 serial)
{
    xdg_wm_base_send_ping(surface()->client()->xdgWmBaseResource(),serial);
}

void LPopupRole::configureC(const LRect &r)
{
    LRect rect = r/compositor()->globalScale();
    xdg_popup_send_configure(resource(), rect.x(), rect.y(), rect.w(), rect.h());
    xdg_surface_send_configure(surface()->imp()->xdgRSurface, LWayland::nextSerial());
}

void LPopupRole::sendPopupDoneEvent()
{
    if (m_imp->dismissed)
        return;

    list<LSurface*>::const_reverse_iterator s = surface()->children().rbegin();
    for (; s!= surface()->children().rend(); s++)
    {
        if ((*s)->popup())
            (*s)->popup()->sendPopupDoneEvent();
    }

    xdg_popup_send_popup_done(resource());
    surface()->imp()->mapped = false;
    surface()->mappingChanged();
    m_imp->dismissed = true;
}


#if LOUVRE_XDG_WM_BASE_VERSION >= 3
void LPopupRole::sendRepositionedEvent(UInt32 token)
{
    if (wl_resource_get_version(resource()) >= 3)
        xdg_popup_send_repositioned(resource(),token);
}
#endif

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
    return m_imp->positioner;
}

LPopupRole::LPopupRolePrivate *LPopupRole::imp() const
{
    return m_imp;
}

void LPopupRole::handleSurfaceCommit()
{

    // Commit inicial para asignar rol y padre
    if (surface()->imp()->pending.role)
    {

        if (surface()->buffer())
        {
            wl_resource_post_error(resource(), XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED, "Given wl_surface already has a buffer attached.");
            return;
        }

        if (surface()->imp()->pendingParent)
        {

            surface()->imp()->setParent(surface()->imp()->pendingParent);
            surface()->imp()->pendingParent = nullptr;
        }
        else
        {
            wl_resource_post_error(resource(), XDG_WM_BASE_ERROR_INVALID_POPUP_PARENT, "xdg_surface.get_popup with invalid popup parent.");
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
    m_imp->positionerBoundsC = bounds;
    m_imp->positionerBoundsS = bounds / compositor()->globalScale();
}

const LRect &LPopupRole::positionerBoundsC() const
{
    return m_imp->positionerBoundsC;
}

const LRect &LPopupRole::positionerBoundsS() const
{
    return m_imp->positionerBoundsS;
}
