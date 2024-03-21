#include <protocols/XdgShell/RXdgSurface.h>
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
#include <LTime.h>

using namespace Louvre;

LPopupRole::LPopupRole(const void *params) :
    LBaseSurfaceRole(((LPopupRole::Params*)params)->popup,
                     ((LPopupRole::Params*)params)->surface,
                       LSurface::Role::Popup),
    LPRIVATE_INIT_UNIQUE(LPopupRole)
{
    imp()->positioner.imp()->data = ((LPopupRole::Params*)params)->positioner->imp()->data;
    imp()->positioner.setUnconstrainedSize(imp()->positioner.size());
}

LPopupRole::~LPopupRole()
{
    if (surface())
        surface()->imp()->setMapped(false);
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

    res.configure(rect.x(), rect.y(), rect.w(), rect.h());
    res.xdgSurfaceRes()->configure(LTime::nextSerial());
}

void LPopupRole::dismiss()
{
    std::list<LSurface*>::const_reverse_iterator s = compositor()->surfaces().rbegin();
    for (; s!= compositor()->surfaces().rend(); s++)
    {
        if ((*s)->popup() && ((*s)->isSubchildOf(surface()) || *s == surface() ))
        {
            if (!imp()->dismissed)
            {
                XdgShell::RXdgPopup *res = (XdgShell::RXdgPopup*)resource();
                res->popupDone();
                imp()->dismissed = true;
            }

            if ((*s) == surface())
                return;
        }
    }
}

const LRect &LPopupRole::windowGeometry() const
{
    return xdgSurfaceResource()->windowGeometry();
}

const LSize &LPopupRole::size() const
{
    return xdgSurfaceResource()->windowGeometry().size();
}

const LPositioner &LPopupRole::positioner() const
{
    return imp()->positioner;
}

void LPopupRole::handleSurfaceCommit(Protocols::Wayland::RSurface::CommitOrigin origin)
{
    L_UNUSED(origin);

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
    if (xdgSurfaceResource()->m_hasPendingWindowGeometry)
    {
        xdgSurfaceResource()->m_hasPendingWindowGeometry = false;
        xdgSurfaceResource()->m_currentWindowGeometry = xdgSurfaceResource()->m_pendingWindowGeometry;
        geometryChanged();
    }
    // Si nunca ha asignado la geometría, usa el tamaño de la superficie
    else if (!xdgSurfaceResource()->m_windowGeometrySet)
    {
        xdgSurfaceResource()->m_currentWindowGeometry = LRect(0, surface()->size());
        geometryChanged();
    }

    // Request configure
    if (!surface()->mapped() && !surface()->buffer())
    {
        configureRequest();
        return;
    }

    // Request unmap
    if (surface()->mapped() && !surface()->buffer())
    {
        surface()->imp()->setMapped(false);
        surface()->imp()->setKeyboardGrabToParent();
        surface()->imp()->setParent(nullptr);
        return;
    }

    // Request map
    if (!surface()->mapped() && surface()->buffer() && surface()->parent())
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
