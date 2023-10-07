#include <protocols/Wayland/private/RSubsurfacePrivate.h>
#include <protocols/Wayland/private/RSurfacePrivate.h>

#include <protocols/Wayland/GSubcompositor.h>

#include <private/LSubsurfaceRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <LCompositor.h>
#include <LLog.h>

struct wl_subsurface_interface subsurface_implementation =
{
    .destroy = &RSubsurface::RSubsurfacePrivate::destroy,
    .set_position = &RSubsurface::RSubsurfacePrivate::set_position,
    .place_above = &RSubsurface::RSubsurfacePrivate::place_above,
    .place_below = &RSubsurface::RSubsurfacePrivate::place_below,
    .set_sync = &RSubsurface::RSubsurfacePrivate::set_sync,
    .set_desync = &RSubsurface::RSubsurfacePrivate::set_desync
};

RSubsurface::RSubsurface
(
    GSubcompositor *subcompositor,
    LSurface *surface,
    LSurface *parent,
    UInt32 id
)
    :LResource
    (
        surface->client(),
        &wl_subsurface_interface,
        subcompositor->version(),
        id,
        &subsurface_implementation,
        &RSubsurface::RSubsurfacePrivate::resource_destroy
    )
{
    m_imp = new RSubsurfacePrivate();

    LSubsurfaceRole::Params subsurfaceRoleParams;
    subsurfaceRoleParams.subsurface = this;
    subsurfaceRoleParams.surface = surface;

    imp()->lSubsurfaceRole = compositor()->createSubsurfaceRoleRequest(&subsurfaceRoleParams);

    // Based on wl_subsurface doc, parent should be applied when parent commits
    surface->imp()->setPendingParent(parent);
    surface->imp()->setPendingRole(imp()->lSubsurfaceRole);
    surface->imp()->applyPendingRole();
}

RSubsurface::~RSubsurface()
{
    // Notify
    compositor()->destroySubsurfaceRoleRequest(imp()->lSubsurfaceRole);
    delete imp()->lSubsurfaceRole;
    delete m_imp;
}

LSubsurfaceRole *RSubsurface::subsurfaceRole() const
{
    return imp()->lSubsurfaceRole;
}
